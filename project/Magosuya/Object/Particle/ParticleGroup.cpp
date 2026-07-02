
#include <numbers>
#include "ParticleGroup.h"
#include "IParticleField.h"
#include "MathFunction.h"
#include "Deltatime.h"
#include "imgui.h"
#include "TextureManager.h"
#include "WindowsAPI.h"

static inline const uint32_t kParticleVertexNum = 4;
static inline const uint32_t kParticleIndexNum = 6;

static inline const uint32_t kRingDivide = 32; // リングの分割数
static inline const float kOutRadius = 1.0f;
static inline const float kInnerRadius = 0.2f;
static inline const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / float(kRingDivide);

ParticleGroup::ParticleGroup(DxCommon* dxCommon) {
	dxCommon_ = dxCommon;
	// インスタンス生成
	vertexBuffer_ = std::make_unique<VertexBuffer<ParticleVertex>>();
	indexBuffer_ = std::make_unique<IndexBuffer<uint32_t>>();
	instancingBuffer_ = std::make_unique<StructuredBuffer<ParticleForGPU>>();
	materialBuffer_ = std::make_unique<MaterialResource>();
}

ParticleGroup::~ParticleGroup() {

}

void ParticleGroup::Initialize(const std::string& name) {
	name_ = name;

	mesh_ = std::make_unique<RingParticleMesh>();
	CreateMesh();

	instancingBuffer_->Initialize(dxCommon_, kMaxParticleNum_);

	materialBuffer_->Initialize(dxCommon_);
	material_.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	material_.uvTransform = Math::MakeIdentity4x4();
	materialBuffer_->Update(material_);

	// PSO設定
	psoDesc_.RootSignatureID = RootSignatureManager::GetInstance ()->GetOrCreateRootSignature (RootSigType::Particle);
	psoDesc_.VS_ID = ShaderManager::GetInstance ()->CompileAndCacheShader (L"Resources/shader/Particle.VS.hlsl", L"vs_6_0");
	psoDesc_.PS_ID = ShaderManager::GetInstance ()->CompileAndCacheShader (L"Resources/shader/Particle.PS.hlsl", L"ps_6_0");
	psoDesc_.InputLayoutID = InputLayoutType::Particle;
	psoDesc_.BlendMode = BlendModeType::Alpha;
	psoDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;	//Depthの書き込みを行わない
	layer_ = 1;
	renderType_ = RenderType::Particle;
}

void ParticleGroup::CreateMesh() {
	if (!mesh_) return;

	std::vector<ParticleVertex> vertices;
	std::vector<uint32_t> indices;

	mesh_->GenerateMesh(vertices, indices);

	vertexBuffer_->Initialize(dxCommon_, vertices);
	vertexBuffer_->Update(vertices);
	indexBuffer_->Initialize(dxCommon_, static_cast<uint32_t>(indices.size()));
	indexBuffer_->Update(indices);
}

// 更新処理
void ParticleGroup::Update(const CameraData& cameraData) {
	std::vector<ParticleForGPU> updateData;
	updateData.reserve(particles_.size());

	for(auto it = particles_.begin(); it != particles_.end();) {
		if(it->currentTime >= it->lifeTime) {
			it = particles_.erase(it);
			continue;
		}

		// 各種フィールドの効果を適用
		it->acceleration = { 0.0f, 0.0f, 0.0f }; // 毎フレーム初期化
		for(size_t i = 0; i < fields_.size(); ++i) {
			fields_[i]->Apply(*it);
		}

		// 物理計算（加速度 -> 速度 -> 位置）
		it->velocity += it->acceleration * kDeltaTime;
		it->transform.translate += it->velocity * kDeltaTime;
		it->currentTime += kDeltaTime;

		// インスタンシング用バッファに詰める処理（省略）
		ParticleForGPU gpuData;

		// スケールと回転（もしあれば）だけで行列を作成（平行移動は一旦原点にする）
		Matrix4x4 world = Math::MakeAffineMatrix(
		it->transform.scale,
		it->transform.rotate,
		{ 0.0f, 0.0f, 0.0f }
		);

		// ビルボード処理：カメラのY軸回転だけを反映させる（Y軸ビルボード）
		if(useBillboard_) {
			Matrix4x4 billboardRotation = cameraData.world;
			billboardRotation.m[3][0] = 0.0f; // 平行移動成分を消す
			billboardRotation.m[3][1] = 0.0f;
			billboardRotation.m[3][2] = 0.0f;
			billboardRotation.m[3][3] = 1.0f;

			gpuData.world = Math::Multiply(world, billboardRotation);
		}
		else {
			// ビルボードを使わない場合はそのまま使う
			gpuData.world = world;
		}

		// 最後にパーティクルの位置を平行移動として適用する
		gpuData.world.m[3][0] = it->transform.translate.x;
		gpuData.world.m[3][1] = it->transform.translate.y;
		gpuData.world.m[3][2] = it->transform.translate.z;
		
		gpuData.WVP = Math::Multiply(gpuData.world, cameraData.vp);

		// カラーの設定と、寿命に応じたアルファフェードアウト
		gpuData.color = it->color;
		float alpha = 1.0f - (it->currentTime / it->lifeTime);
		gpuData.color.w = alpha; // w成分（Alpha）を徐々に透明にする

		// ベクターに追加
		updateData.push_back(gpuData);

		++it;
	}

	// まとめてGPUバッファに転送！
	instancingBuffer_->Update(updateData);
	materialBuffer_->Update(material_);
}

void ParticleGroup::Draw() {
	RenderCommand cmd;

	// PSOの設定
	cmd.rootSignatureID = psoDesc_.RootSignatureID;
	cmd.psoDesc = psoDesc_;

	// メッシュ情報
	cmd.vbViews[0] = vertexBuffer_->GetView();
	cmd.ibv = indexBuffer_->GetView();

	if (mesh_) {
		cmd.indexCount = mesh_->GetIndexCount();
	} else {
		cmd.indexCount = kParticleIndexNum;
	}

	// 現在のパーティクル数をインスタンス数として設定！
	cmd.instanceCount = static_cast<UINT>(particles_.size());

	// 定数バッファのアドレス
	cmd.binds[0].type = BindingType::SRV_Table;
	cmd.binds[0].descriptorHandle = instancingBuffer_->GetSRVHandle();

	cmd.binds[1].type = BindingType::CBV;
	cmd.binds[1].gpuAddress = materialBuffer_.get()->GetGPUVirtualAddress();

	cmd.binds[2].type = BindingType::SRV_Table;
	cmd.binds[2].descriptorHandle = textureHandle_;

	// 透明
	cmd.layer = layer_;

	// 描画タイプ
	cmd.renderType = renderType_;

	// コマンドを投げる
	RenderSystem::GetInstance()->PushCommand(cmd);
}

void ParticleGroup::ImGui() {
#ifdef USEIMGUI
	// ユニークラベル作成
	std::string label = "##" + name_;

	// 名前の変更
	char nameBuffer[128];
	snprintf(nameBuffer, sizeof(nameBuffer), "%s", name_.c_str());
	if(ImGui::InputText("Group Name", nameBuffer, sizeof(nameBuffer))) {
		name_ = nameBuffer;
	}

	// テクスチャロード用の静的バッファ（ImGui表示用）
	static char texPathBuffer[256] = "";
	static std::string s_lastGroupName = "";
	if (s_lastGroupName != name_ || texPathBuffer[0] == '\0') {
		snprintf(texPathBuffer, sizeof(texPathBuffer), "%s", texInfo_.filePath.c_str());
		s_lastGroupName = name_;
	}

	// 使っているテクスチャ表示
	ImGui::Text("Texture: %s", texInfo_.filePath.empty() ? "None" : texInfo_.filePath.c_str());

	ImGui::SameLine(); // ボタンを横並びにする
	if (ImGui::Button(("Open" + label).c_str())) {
		// ダイアログを開いて絶対パスを取得
		std::string absPath = FileUtils::OpenFileDialog();
		if (!absPath.empty()) {
			// 自作エンジンで扱いやすいように「Resources/...」からの相対パスに変換
			std::string relPath = FileUtils::GetRelativePath(absPath);

			int newIndex = TextureManager::GetInstance()->LoadTexture(relPath);
			if (newIndex >= 0) {
				// テクスチャ情報を更新
				SetTextureIndex(newIndex);
				// ★パスを直接メンバー変数に記憶しちゃう！
				texInfo_.filePath = relPath; 
				snprintf(texPathBuffer, sizeof(texPathBuffer), "%s", relPath.c_str()); // バッファも同期する
			}
		}
	}

	// 手動パス入力ロード機能
	if (ImGui::InputText(("Texture Path" + label).c_str(), texPathBuffer, sizeof(texPathBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
		int newIndex = TextureManager::GetInstance()->LoadTexture(texPathBuffer);
		if (newIndex >= 0) {
			SetTextureIndex(newIndex);
			texInfo_.filePath = texPathBuffer;
		}
	}
	ImGui::SameLine();
	if (ImGui::Button(("Load##Tex" + label).c_str())) {
		int newIndex = TextureManager::GetInstance()->LoadTexture(texPathBuffer);
		if (newIndex >= 0) {
			SetTextureIndex(newIndex);
			texInfo_.filePath = texPathBuffer;
		}
	}

	ImGui::Separator();

	ImGui::Text("Particle Behavior");

	// --- Scale ---
	ImGui::Checkbox(("Random Scale" + label).c_str(), &behavior_.isRandomScale);
	if (behavior_.isRandomScale) {
		ImGui::DragFloat3(("Scale Min" + label).c_str(), &behavior_.minScale.x, 0.01f);
		ImGui::DragFloat3(("Scale Max" + label).c_str(), &behavior_.maxScale.x, 0.01f);
	}
	else {
		ImGui::DragFloat3(("Scale (Fixed)" + label).c_str(), &behavior_.transform.scale.x, 0.01f);
		behavior_.minScale = behavior_.transform.scale;
	}

	// --- Rotate ---
	ImGui::Checkbox(("Random Rotate" + label).c_str(), &behavior_.isRandomRotate);
	if (behavior_.isRandomRotate) {
		ImGui::DragFloat3(("Rotate Min" + label).c_str(), &behavior_.minRotate.x, 0.01f);
		ImGui::DragFloat3(("Rotate Max" + label).c_str(), &behavior_.maxRotate.x, 0.01f);
	} 
	else {
		ImGui::DragFloat3(("Rotate (Fixed)" + label).c_str(), &behavior_.transform.rotate.x, 0.01f, 0.0f, 3.14f);
		behavior_.minRotate = behavior_.transform.rotate;
	}

	// --- Translate ---
	ImGui::Checkbox(("Random Translate" + label).c_str(), &behavior_.isRandomTranslate);
	if(behavior_.isRandomTranslate) {
		ImGui::DragFloat3(("Translate Min" + label).c_str(), &behavior_.minTranslate.x, 0.01f);
		ImGui::DragFloat3(("Translate Min" + label).c_str(), &behavior_.maxTranslate.x, 0.01f);
	}
	else {
		ImGui::DragFloat3(("Translate (Fixed)" + label).c_str(), &behavior_.transform.translate.x, 0.01f);
		behavior_.minTranslate = behavior_.transform.translate;
	}

	// --- Velocity ---
	ImGui::Checkbox(("Random Velocity" + label).c_str(), &behavior_.isRandomVelocity);
	if(behavior_.isRandomVelocity) {
		ImGui::DragFloat3(("Velocity Min" + label).c_str(), &behavior_.minVelocity.x, 0.01f);
		ImGui::DragFloat3(("Velocity Max" + label).c_str(), &behavior_.maxVelocity.x, 0.01f);
	} 
	else {
		ImGui::DragFloat3(("Velocity (Fixed)" + label).c_str(), &behavior_.velocity.x, 0.01f);
		behavior_.minVelocity = behavior_.velocity;
	}

	// --- Color ---
	ImGui::Checkbox(("Random Color" + label).c_str(), &behavior_.isRandomColor);
	if(behavior_.isRandomColor) {
		ImGui::ColorEdit4(("Color Min" + label).c_str(), &behavior_.minColor.x);
		ImGui::ColorEdit4(("Color Max" + label).c_str(), &behavior_.maxColor.x);
	} 
	else {
		ImGui::ColorEdit4(("Color (Fixed)" + label).c_str(), &behavior_.maxColor.x);
	}

	// --- LifeTime ---
	ImGui::Checkbox(("Random LifeTime" + label).c_str(), &behavior_.isRandomLifeTime);
	if(behavior_.isRandomLifeTime) {
		ImGui::DragFloat2(("LifeTime (Min/Max)" + label).c_str(), &behavior_.minLifeTime, 0.05f, 0.0f, 10.0f);
	} 
	else {
		ImGui::DragFloat(("LifeTime (Fixed)" + label).c_str(), &behavior_.minLifeTime, 0.05f, 0.0f, 10.0f);
	}

	ImGui::Separator();

	// --- Particle Shape ---
	if (mesh_) {
		std::string currentShape = mesh_->GetName();
		if (ImGui::BeginCombo(("Primitive Type" + label).c_str(), currentShape.c_str())) {
			if (ImGui::Selectable("Quad", currentShape == "Quad")) {
				mesh_ = std::make_unique<QuadParticleMesh>();
				CreateMesh();
			}
			if (ImGui::Selectable("Ring", currentShape == "Ring")) {
				mesh_ = std::make_unique<RingParticleMesh>();
				CreateMesh();
			}
			if (ImGui::Selectable("Cylinder", currentShape == "Cylinder")) {
				mesh_ = std::make_unique<CylinderParticleMesh>();
				CreateMesh();
			}
			ImGui::EndCombo();
		}
	}

	ImGui::Separator();
	
	// --- Billboard --- //
	ImGui::Checkbox(("Use Billboard" + label).c_str(), &useBillboard_);
#endif
}

void ParticleGroup::AddParticle(const ParticleData& particle) {
	if(particles_.size() < kMaxParticleNum_) {
		particles_.push_back(particle);
	}
}

void ParticleGroup::AddField(IParticleField* field) {
	if (field != nullptr) {
		fields_.push_back(field);
	}
}

bool ParticleGroup::HasField(IParticleField* field) {
	for (size_t i = 0; i < fields_.size(); ++i) {
		if (fields_[i] == field) return true;
	}
	return false;
}

void ParticleGroup::RemoveField(IParticleField* field) {
	for (auto it = fields_.begin(); it != fields_.end(); ) {
		if (*it == field) {
			it = fields_.erase(it); // 見つけたら削除
		} else {
			++it;
		}
	}
}

void ParticleGroup::SaveConfig(json& jsonOut) const {
	// 名前
	jsonOut["name"] = name_;

	// テクスチャ用の情報
	jsonOut["texturePath"] = texInfo_.filePath;
	
	// behaviorのパラメータを詰める
	auto& b = jsonOut["behavior"];
	// Scale
	b["isRandomScale"] = behavior_.isRandomScale;
	b["minScale"] = { behavior_.minScale.x, behavior_.minScale.y, behavior_.minScale.z };
	b["maxScale"] = { behavior_.maxScale.x, behavior_.maxScale.y, behavior_.maxScale.z };
	b["scale"] = { behavior_.transform.scale.x, behavior_.transform.scale.y, behavior_.transform.scale.z };

	// Rotate
	b["isRandomRotate"] = behavior_.isRandomRotate;
	b["minRotate"] = { behavior_.minRotate.x, behavior_.minRotate.y, behavior_.minRotate.z };
	b["maxRotate"] = { behavior_.maxRotate.x, behavior_.maxRotate.y, behavior_.maxRotate.z };
	b["rotate"] = { behavior_.transform.rotate.x, behavior_.transform.rotate.y, behavior_.transform.rotate.z };

	// Translate
	b["isRandomTranslate"] = behavior_.isRandomTranslate;
	b["minTranslate"] = { behavior_.minTranslate.x, behavior_.minTranslate.y, behavior_.minTranslate.z };
	b["maxTranslate"] = { behavior_.maxTranslate.x, behavior_.maxTranslate.y, behavior_.maxTranslate.z };
	b["translate"] = { behavior_.transform.translate.x, behavior_.transform.translate.y, behavior_.transform.translate.z };

	// Velocity
	b["isRandomVelocity"] = behavior_.isRandomVelocity;
	b["minVelocity"] = { behavior_.minVelocity.x, behavior_.minVelocity.y, behavior_.minVelocity.z };
	b["maxVelocity"] = { behavior_.maxVelocity.x, behavior_.maxVelocity.y, behavior_.maxVelocity.z };
	b["velocity"] = { behavior_.velocity.x, behavior_.velocity.y, behavior_.velocity.z };

	// Color
	b["isRandomColor"] = behavior_.isRandomColor;
	b["minColor"] = { behavior_.minColor.x, behavior_.minColor.y, behavior_.minColor.z, behavior_.minColor.w };
	b["maxColor"] = { behavior_.maxColor.x, behavior_.maxColor.y, behavior_.maxColor.z, behavior_.maxColor.w };

	// LifeTime
	b["minLifeTime"] = behavior_.minLifeTime;
	b["maxLifeTime"] = behavior_.maxLifeTime;

	// useBillboard
	jsonOut["useBillboard"] = useBillboard_;

	if (mesh_) {
		jsonOut["primitiveType"] = mesh_->GetName();
	}
}

void ParticleGroup::LoadConfig(const json& jsonIn) {
	// name
	if(jsonIn.contains("name")) name_ = jsonIn["name"];

	// テクスチャの復元
	if(jsonIn.contains("texturePath")) {
		texInfo_.filePath = jsonIn["texturePath"].get<std::string>();
		// パスを基にロードし直して、新しいインデックス（int）を受け取る！
		texInfo_.index = TextureManager::GetInstance()->LoadTexture(texInfo_.filePath);

		// GPUハンドルもマネージャーから引き直して適用
		textureHandle_ = TextureManager::GetInstance()->GetTextureHandle(texInfo_.index);
	}

	if(jsonIn.contains("behavior")) {
		auto& b = jsonIn["behavior"];

		// Scale
		behavior_.isRandomScale = b["isRandomScale"];
		behavior_.minScale.x = b["minScale"][0];
		behavior_.minScale.y = b["minScale"][1];
		behavior_.minScale.z = b["minScale"][2];
		behavior_.maxScale.x = b["maxScale"][0];
		behavior_.maxScale.y = b["maxScale"][1];
		behavior_.maxScale.z = b["maxScale"][2];
		behavior_.transform.scale.x = b["scale"][0];
		behavior_.transform.scale.y = b["scale"][1];
		behavior_.transform.scale.z = b["scale"][2];

		// Rotate
		behavior_.isRandomRotate = b["isRandomRotate"];
		behavior_.minRotate.x = b["minRotate"][0];
		behavior_.minRotate.y = b["minRotate"][1];
		behavior_.minRotate.z = b["minRotate"][2];
		behavior_.maxRotate.x = b["maxRotate"][0];
		behavior_.maxRotate.y = b["maxRotate"][1];
		behavior_.maxRotate.z = b["maxRotate"][2];
		behavior_.transform.rotate.x = b["rotate"][0];
		behavior_.transform.rotate.y = b["rotate"][1];
		behavior_.transform.rotate.z = b["rotate"][2];

		// Translate
		behavior_.isRandomTranslate = b["isRandomTranslate"];
		behavior_.minTranslate.x = b["minTranslate"][0];
		behavior_.minTranslate.y = b["minTranslate"][1];
		behavior_.minTranslate.z = b["minTranslate"][2];
		behavior_.maxTranslate.x = b["maxTranslate"][0];
		behavior_.maxTranslate.y = b["maxTranslate"][1];
		behavior_.maxTranslate.z = b["maxTranslate"][2];
		behavior_.transform.translate.x = b["translate"][0];
		behavior_.transform.translate.y = b["translate"][1];
		behavior_.transform.translate.z = b["translate"][2];

		// Velocity
		behavior_.isRandomVelocity = b["isRandomVelocity"];
		behavior_.minVelocity.x = b["minVelocity"][0];
		behavior_.minVelocity.y = b["minVelocity"][1];
		behavior_.minVelocity.z = b["minVelocity"][2];
		behavior_.maxVelocity.x = b["maxVelocity"][0];
		behavior_.maxVelocity.y = b["maxVelocity"][1];
		behavior_.maxVelocity.z = b["maxVelocity"][2];
		behavior_.velocity.x = b["velocity"][0];
		behavior_.velocity.y = b["velocity"][1];
		behavior_.velocity.z = b["velocity"][2];

		// Color
		behavior_.isRandomColor = b["isRandomColor"];
		behavior_.minColor.x = b["minColor"][0];
		behavior_.minColor.y = b["minColor"][1];
		behavior_.minColor.z = b["minColor"][2];
		behavior_.minColor.w = b["minColor"][3];
		behavior_.maxColor.x = b["maxColor"][0];
		behavior_.maxColor.y = b["maxColor"][1];
		behavior_.maxColor.z = b["maxColor"][2];
		behavior_.maxColor.w = b["maxColor"][3];

		// LifeTime
		behavior_.minLifeTime = b["minLifeTime"];
		behavior_.maxLifeTime = b["maxLifeTime"];
	}

	if(jsonIn.contains("useBillboard")) useBillboard_ = jsonIn["useBillboard"];

	if (jsonIn.contains("primitiveType")) {
		std::string primType = jsonIn["primitiveType"];
		if (primType == "Quad") {
			mesh_ = std::make_unique<QuadParticleMesh>();
		} else if (primType == "Cylinder") {
			mesh_ = std::make_unique<CylinderParticleMesh>();
		} else {
			mesh_ = std::make_unique<RingParticleMesh>();
		}
		CreateMesh();
	} else {
		mesh_ = std::make_unique<RingParticleMesh>();
		CreateMesh();
	}
}

void ParticleGroup::SetTextureIndex(int index) {
	texInfo_.index = index; 
	// ハンドルもグループ内部で自動的に引き直して保持させる
	textureHandle_ = TextureManager::GetInstance()->GetTextureHandle(index);
}
