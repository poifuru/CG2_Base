#include "ParticleGroup.h"
#include "IParticleField.h"
#include "MathFunction.h"
#include "Deltatime.h"

static inline const uint32_t kParticleVertexNum = 4;
static inline const uint32_t kParticleIndexNum = 6;

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

	// バッファ初期化
	std::vector<ParticleVertex> vertices(kParticleVertexNum);
	vertices[0] = {	// 左上
		{-1.0f, 1.0f, 0.0f, 1.0f},
		{0.0f, 0.0f},
	};
	vertices[1] = {	// 右上
		{1.0f, 1.0f, 0.0f, 1.0f},
		{1.0f, 0.0f},
	};
	vertices[2] = {	// 左下
		{-1.0f, -1.0f, 0.0f, 1.0f},
		{0.0f, 1.0f},
	};
	vertices[3] = {	// 右下
		{1.0f, -1.0f, 0.0f, 1.0f},
		{1.0f, 1.0f},
	};
	vertexBuffer_->Initialize(dxCommon_, vertices);
	vertexBuffer_->Update(vertices);

	std::vector<uint32_t> indices = {
	0, 1, 2,
	1, 3, 2
	};
	indexBuffer_->Initialize(dxCommon_, kParticleIndexNum);
	indexBuffer_->Update(indices);

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
	psoDesc_.BlendMode = BlendModeType::Additive;
	psoDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;	//Depthの書き込みを行わない
	layer_ = 1;
	renderType_ = RenderType::Particle;
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

		// 行列作成
		Matrix4x4 world = Math::MakeAffineMatrix(
			it->transform.scale,
			it->transform.rotate,
			it->transform.translate
		);

		// ビルボード処理：カメラの回転成分（3x3部分）を乗算する
		if(useBillboard_) {
			Matrix4x4 billboardRotation = cameraData.world;
			billboardRotation.m[3][0] = 0.0f; // 平行移動成分を消す
			billboardRotation.m[3][1] = 0.0f;
			billboardRotation.m[3][2] = 0.0f;
			billboardRotation.m[3][3] = 1.0f;

			gpuData.world = Math::Multiply(world, billboardRotation);
		}
		else {
			// ビルボードを使わない場合は通常のアフィン行列をそのまま使う
			gpuData.world = world;
		}
		
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
	cmd.indexCount = kParticleIndexNum;

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