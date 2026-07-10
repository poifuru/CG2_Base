
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

// 静的メンバの実体定義
ComPtr<ID3D12RootSignature> ParticleGroup::sInitRootSignature_ = nullptr;
ComPtr<ID3D12PipelineState> ParticleGroup::sInitPSO_ = nullptr;
ComPtr<ID3D12RootSignature> ParticleGroup::sEmitRootSignature_ = nullptr;
ComPtr<ID3D12PipelineState> ParticleGroup::sEmitPSO_ = nullptr;
ComPtr<ID3D12RootSignature> ParticleGroup::sUpdateRootSignature_ = nullptr;
ComPtr<ID3D12PipelineState> ParticleGroup::sUpdatePSO_ = nullptr;
ComPtr<ID3D12RootSignature> ParticleGroup::sClearRootSignature_ = nullptr;
ComPtr<ID3D12PipelineState> ParticleGroup::sClearPSO_ = nullptr;

ParticleGroup::ParticleGroup(DxCommon* dxCommon) {
	dxCommon_ = dxCommon;
	// インスタンス生成
	vertexBuffer_ = std::make_unique<VertexBuffer<ParticleVertex>>();
	indexBuffer_ = std::make_unique<IndexBuffer<uint32_t>>();
	materialBuffer_ = std::make_unique<MaterialResource>();

	poolBuffer_ = std::make_unique<StructuredBuffer<GPUParticle>>();
	freeListBuffer_ = std::make_unique<StructuredBuffer<uint32_t>>();
	freeListCounterBuffer_ = std::make_unique<StructuredBuffer<int32_t>>();
	drawParticlesBuffer_ = std::make_unique<StructuredBuffer<ParticleForGPU>>();
	drawArgumentsBuffer_ = std::make_unique<StructuredBuffer<D3D12_DRAW_INDEXED_ARGUMENTS>>();

	emitRequestsBuffer_ = std::make_unique<StructuredBuffer<ParticleEmitRequest>>();
	updateParamsBuffer_ = std::make_unique<ConstantBuffer<UpdateParams>>();
}

ParticleGroup::~ParticleGroup() {

}

void ParticleGroup::Initialize(const std::string& name) {
	name_ = name;

	mesh_ = std::make_unique<RingParticleMesh>();
	CreateMesh();

	// 各種バッファの初期化
	poolBuffer_->InitializeUAV(dxCommon_, kMaxParticleNum_);
	freeListBuffer_->InitializeUAV(dxCommon_, kMaxParticleNum_);
	freeListCounterBuffer_->InitializeUAV(dxCommon_, 1);
	drawParticlesBuffer_->InitializeUAV(dxCommon_, kMaxParticleNum_);
	drawArgumentsBuffer_->InitializeUAV(dxCommon_, 1);

	// CPUからの転送バッファの初期化
	emitRequestsBuffer_->Initialize(dxCommon_, kMaxParticleNum_);
	updateParamsBuffer_->Initialize(dxCommon_);

	materialBuffer_->Initialize(dxCommon_);
	material_.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	material_.uvTransform = Math::MakeIdentity4x4();
	materialBuffer_->Update(material_);

	// PSO設定
	psoDesc_.RootSignatureID = RootSignatureManager::GetInstance ()->GetOrCreateRootSignature (RootSigType::Particle);
	psoDesc_.VS_ID = ShaderManager::GetInstance ()->CompileAndCacheShader (L"Resources/shader/GPUParticle.VS.hlsl", L"vs_6_0");
	psoDesc_.PS_ID = ShaderManager::GetInstance ()->CompileAndCacheShader (L"Resources/shader/Particle.PS.hlsl", L"ps_6_0");
	psoDesc_.InputLayoutID = InputLayoutType::Particle;
	psoDesc_.BlendMode = BlendModeType::Alpha;
	psoDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;	//Depthの書き込みを行わない
	layer_ = 1;
	renderType_ = RenderType::Particle;

	// CS用の静的パイプラインが未初期化なら初期化する
	if (!sInitPSO_) {
		InitializeComputePipelines(dxCommon_);
	}

	// GPUバッファの初期データ設定をCSで行う
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
	commandList->SetPipelineState(sInitPSO_.Get());
	commandList->SetComputeRootSignature(sInitRootSignature_.Get());

	commandList->SetComputeRootUnorderedAccessView(0, freeListBuffer_->GetResource()->GetGPUVirtualAddress());
	commandList->SetComputeRootUnorderedAccessView(1, freeListCounterBuffer_->GetResource()->GetGPUVirtualAddress());
	commandList->SetComputeRootUnorderedAccessView(2, drawArgumentsBuffer_->GetResource()->GetGPUVirtualAddress());

	// InitParams (indexCount) の設定
	uint32_t indexCount = mesh_ ? mesh_->GetIndexCount() : kParticleIndexNum;
	commandList->SetComputeRoot32BitConstants(3, 1, &indexCount, 0);

	UINT numGroups = (kMaxParticleNum_ + 255) / 256;
	commandList->Dispatch(numGroups, 1, 1);

	// 初期化CS完了を保証するためのUAVバリア
	D3D12_RESOURCE_BARRIER barriers[3] = {};
	barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barriers[0].UAV.pResource = freeListBuffer_->GetResource();
	barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barriers[1].UAV.pResource = freeListCounterBuffer_->GetResource();
	barriers[2].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barriers[2].UAV.pResource = drawArgumentsBuffer_->GetResource();
	commandList->ResourceBarrier(3, barriers);
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
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	// 1. 各バッファを UAV 状態に遷移
	D3D12_RESOURCE_BARRIER beforeBarriers[2] = {};
	beforeBarriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	beforeBarriers[0].Transition.pResource = drawParticlesBuffer_->GetResource();
	beforeBarriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	beforeBarriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	beforeBarriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	beforeBarriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	beforeBarriers[1].Transition.pResource = drawArgumentsBuffer_->GetResource();
	beforeBarriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
	beforeBarriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	beforeBarriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	commandList->ResourceBarrier(2, beforeBarriers);

	// 2. Clear パス (InstanceCount を 0 にリセット)
	commandList->SetPipelineState(sClearPSO_.Get());
	commandList->SetComputeRootSignature(sClearRootSignature_.Get());
	commandList->SetComputeRootUnorderedAccessView(0, drawArgumentsBuffer_->GetResource()->GetGPUVirtualAddress());
	commandList->Dispatch(1, 1, 1);

	// UAVバリア (Clear完了待ち)
	D3D12_RESOURCE_BARRIER clearBarrier = {};
	clearBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	clearBarrier.UAV.pResource = drawArgumentsBuffer_->GetResource();
	commandList->ResourceBarrier(1, &clearBarrier);

	// 3. Emit パス (新規追加されたパーティクルの発生要求を実行)
	if (!emitRequests_.empty()) {
		emitRequestsBuffer_->Update(emitRequests_);

		commandList->SetPipelineState(sEmitPSO_.Get());
		commandList->SetComputeRootSignature(sEmitRootSignature_.Get());
		commandList->SetComputeRootUnorderedAccessView(0, poolBuffer_->GetResource()->GetGPUVirtualAddress());
		commandList->SetComputeRootUnorderedAccessView(1, freeListBuffer_->GetResource()->GetGPUVirtualAddress());
		commandList->SetComputeRootUnorderedAccessView(2, freeListCounterBuffer_->GetResource()->GetGPUVirtualAddress());
		commandList->SetComputeRootShaderResourceView(3, emitRequestsBuffer_->GetResource()->GetGPUVirtualAddress());
		
		uint32_t emitCount = static_cast<uint32_t>(emitRequests_.size());
		commandList->SetComputeRoot32BitConstants(4, 1, &emitCount, 0);

		UINT numGroups = (emitCount + 255) / 256;
		commandList->Dispatch(numGroups, 1, 1);

		// UAVバリア (Emit完了待ち)
		D3D12_RESOURCE_BARRIER emitBarriers[3] = {};
		emitBarriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		emitBarriers[0].UAV.pResource = poolBuffer_->GetResource();
		emitBarriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		emitBarriers[1].UAV.pResource = freeListBuffer_->GetResource();
		emitBarriers[2].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		emitBarriers[2].UAV.pResource = freeListCounterBuffer_->GetResource();
		commandList->ResourceBarrier(3, emitBarriers);

		emitRequests_.clear();
	}

	// 4. Update パス (物理更新 & 描画用バッファへの詰め直し)
	UpdateParams params;
	params.cameraWorld = cameraData.world;
	params.vp = cameraData.vp;
	params.deltaTime = kDeltaTime;
	params.useBillboard = useBillboard_ ? 1 : 0;
	updateParamsBuffer_->Update(params);

	commandList->SetPipelineState(sUpdatePSO_.Get());
	commandList->SetComputeRootSignature(sUpdateRootSignature_.Get());
	commandList->SetComputeRootUnorderedAccessView(0, poolBuffer_->GetResource()->GetGPUVirtualAddress());
	commandList->SetComputeRootUnorderedAccessView(1, freeListBuffer_->GetResource()->GetGPUVirtualAddress());
	commandList->SetComputeRootUnorderedAccessView(2, freeListCounterBuffer_->GetResource()->GetGPUVirtualAddress());
	commandList->SetComputeRootUnorderedAccessView(3, drawParticlesBuffer_->GetResource()->GetGPUVirtualAddress());
	commandList->SetComputeRootUnorderedAccessView(4, drawArgumentsBuffer_->GetResource()->GetGPUVirtualAddress());
	commandList->SetComputeRootConstantBufferView(5, updateParamsBuffer_->GetGPUVirtualAddress());

	UINT numGroups = (kMaxParticleNum_ + 255) / 256;
	commandList->Dispatch(numGroups, 1, 1);

	// 5. 描画リソースとしてバインドするためのステート遷移バリア
	D3D12_RESOURCE_BARRIER afterBarriers[2] = {};
	afterBarriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	afterBarriers[0].Transition.pResource = drawParticlesBuffer_->GetResource();
	afterBarriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	afterBarriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	afterBarriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	afterBarriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	afterBarriers[1].Transition.pResource = drawArgumentsBuffer_->GetResource();
	afterBarriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	afterBarriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
	afterBarriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	commandList->ResourceBarrier(2, afterBarriers);

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

	// 間接描画を有効にする！
	cmd.useIndirect = true;
	cmd.indirectArgumentBuffer = drawArgumentsBuffer_->GetResource();

	// 定数バッファのアドレス
	cmd.binds[0].type = BindingType::SRV_Table;
	cmd.binds[0].descriptorHandle = drawParticlesBuffer_->GetSRVHandle();

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
	if(emitRequests_.size() < kMaxParticleNum_) {
		ParticleEmitRequest req;
		req.position = particle.transform.translate;
		req.velocity = particle.velocity;
		req.color = particle.color;
		req.lifeTime = particle.lifeTime;
		req.scale = particle.transform.scale;
		req.rotate = particle.transform.rotate;
		emitRequests_.push_back(req);
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

void ParticleGroup::InitializeComputePipelines(DxCommon* dxCommon) {
	ID3D12Device* device = dxCommon->GetDevice();

	// 1. Init CS
	{
		D3D12_ROOT_PARAMETER rootParameters[4] = {};
		// u0
		rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
		rootParameters[0].Descriptor.ShaderRegister = 0;
		rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		// u1
		rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
		rootParameters[1].Descriptor.ShaderRegister = 1;
		rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		// u2
		rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
		rootParameters[2].Descriptor.ShaderRegister = 2;
		rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		// b0 (Root Constants)
		rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		rootParameters[3].Constants.ShaderRegister = 0;
		rootParameters[3].Constants.RegisterSpace = 0;
		rootParameters[3].Constants.Num32BitValues = 1;
		rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
		rsDesc.NumParameters = _countof(rootParameters);
		rsDesc.pParameters = rootParameters;
		rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

		ComPtr<ID3DBlob> signatureBlob = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
		assert(SUCCEEDED(hr));
		device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&sInitRootSignature_));

		uint32_t csShaderID = ShaderManager::GetInstance()->CompileAndCacheShader(L"Resources/shader/GPUParticleInit.CS.hlsl", L"cs_6_0");
		D3D12_SHADER_BYTECODE csBytecode = ShaderManager::GetInstance()->GetShaderBytecode(csShaderID);

		D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.pRootSignature = sInitRootSignature_.Get();
		psoDesc.CS = csBytecode;
		psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&sInitPSO_));
	}

	// 2. Emit CS
	{
		D3D12_ROOT_PARAMETER rootParameters[5] = {};
		// u0
		rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
		rootParameters[0].Descriptor.ShaderRegister = 0;
		rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		// u1
		rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
		rootParameters[1].Descriptor.ShaderRegister = 1;
		rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		// u2
		rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
		rootParameters[2].Descriptor.ShaderRegister = 2;
		rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		// t0
		rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		rootParameters[3].Descriptor.ShaderRegister = 0;
		rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		// b0 (Root Constants)
		rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		rootParameters[4].Constants.ShaderRegister = 0;
		rootParameters[4].Constants.RegisterSpace = 0;
		rootParameters[4].Constants.Num32BitValues = 1;
		rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
		rsDesc.NumParameters = _countof(rootParameters);
		rsDesc.pParameters = rootParameters;
		rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

		ComPtr<ID3DBlob> signatureBlob = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
		assert(SUCCEEDED(hr));
		device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&sEmitRootSignature_));

		uint32_t csShaderID = ShaderManager::GetInstance()->CompileAndCacheShader(L"Resources/shader/GPUParticleEmit.CS.hlsl", L"cs_6_0");
		D3D12_SHADER_BYTECODE csBytecode = ShaderManager::GetInstance()->GetShaderBytecode(csShaderID);

		D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.pRootSignature = sEmitRootSignature_.Get();
		psoDesc.CS = csBytecode;
		psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&sEmitPSO_));
	}

	// 3. Clear CS
	{
		D3D12_ROOT_PARAMETER rootParameters[1] = {};
		// u0
		rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
		rootParameters[0].Descriptor.ShaderRegister = 0;
		rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
		rsDesc.NumParameters = _countof(rootParameters);
		rsDesc.pParameters = rootParameters;
		rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

		ComPtr<ID3DBlob> signatureBlob = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
		assert(SUCCEEDED(hr));
		device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&sClearRootSignature_));

		uint32_t csShaderID = ShaderManager::GetInstance()->CompileAndCacheShader(L"Resources/shader/GPUParticleClear.CS.hlsl", L"cs_6_0");
		D3D12_SHADER_BYTECODE csBytecode = ShaderManager::GetInstance()->GetShaderBytecode(csShaderID);

		D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.pRootSignature = sClearRootSignature_.Get();
		psoDesc.CS = csBytecode;
		psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&sClearPSO_));
	}

	// 4. Update CS
	{
		D3D12_ROOT_PARAMETER rootParameters[6] = {};
		// u0
		rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
		rootParameters[0].Descriptor.ShaderRegister = 0;
		rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		// u1
		rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
		rootParameters[1].Descriptor.ShaderRegister = 1;
		rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		// u2
		rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
		rootParameters[2].Descriptor.ShaderRegister = 2;
		rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		// u3
		rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
		rootParameters[3].Descriptor.ShaderRegister = 3;
		rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		// u4
		rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
		rootParameters[4].Descriptor.ShaderRegister = 4;
		rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		// b0
		rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParameters[5].Descriptor.ShaderRegister = 0;
		rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
		rsDesc.NumParameters = _countof(rootParameters);
		rsDesc.pParameters = rootParameters;
		rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

		ComPtr<ID3DBlob> signatureBlob = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
		assert(SUCCEEDED(hr));
		device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&sUpdateRootSignature_));

		uint32_t csShaderID = ShaderManager::GetInstance()->CompileAndCacheShader(L"Resources/shader/GPUParticleUpdate.CS.hlsl", L"cs_6_0");
		D3D12_SHADER_BYTECODE csBytecode = ShaderManager::GetInstance()->GetShaderBytecode(csShaderID);

		D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.pRootSignature = sUpdateRootSignature_.Get();
		psoDesc.CS = csBytecode;
		psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&sUpdatePSO_));
	}
}
