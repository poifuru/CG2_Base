#include "ParticleRenderer.h"
#include "MathFunction.h"
#include "DxCommon.h"
#include "SRVManager.h"

void ParticleRenderer::Initialize(ParticleType type) {
	//ParticleTypeで初期化時の内容を変える
	switch(type) {
	case::ParticleType::Standard:
		//頂点バッファ作成
		data_->vertexBuffer = DxCommon::GetInstance()->CreateBufferResource(sizeof(VertexData) * 4);
		data_->vertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&data_->vertexData_));
		data_->vbView.BufferLocation = data_->vertexBuffer->GetGPUVirtualAddress();
		data_->vbView.SizeInBytes = sizeof(VertexData) * 4;
		data_->vbView.StrideInBytes = sizeof(VertexData);

		//インデックスバッファ作成
		data_->indexBuffer = DxCommon::GetInstance()->CreateBufferResource(sizeof(uint32_t) * 6);
		data_->indexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&data_->indexData_));
		data_->ibView.BufferLocation = data_->indexBuffer->GetGPUVirtualAddress();
		data_->ibView.SizeInBytes = sizeof(uint32_t) * 6;
		data_->ibView.Format = DXGI_FORMAT_R32_UINT;

		//Instancing用の行列データ
		data_->instancingBuffer_ = DxCommon::GetInstance()->CreateBufferResource(sizeof(ParticleForGPU) * data_->kMaxParticleNum);
		data_->instancingBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&data_->instancingData_));
		//マテリアルデータ
		data_->materialBuffer_ = DxCommon::GetInstance()->CreateBufferResource(sizeof(Material) * data_->kMaxParticleNum);
		data_->materialBuffer_->Map(0, nullptr, reinterpret_cast<void**> (&data_->materialData_));
		for(uint32_t i = 0; i < data_->kMaxParticleNum; ++i) {
			data_->instancingData_[i].WVP = Math::MakeIdentity4x4();
			data_->instancingData_[i].World = Math::MakeIdentity4x4();
			data_->instancingData_[i].color = { 1.0f, 1.0f, 1.0f, 1.0f };
			data_->materialData_[i].color = { 1.0f, 1.0f, 1.0f, 1.0f };
			data_->materialData_[i].enableLighting = LightReflectionModel::None;
			data_->materialData_[i].uvTranform = Math::MakeIdentity4x4();
		}

		//vertexData_に書き込み
		//左上
		data_->vertexData_[0] = { { -1.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } };
		//右上
		data_->vertexData_[1] = { { 1.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } };
		//左下
		data_->vertexData_[2] = { { -1.0f, -1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } };
		//右下
		data_->vertexData_[3] = { { 1.0f, -1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } };

		//indexData_に書き込み
		data_->indexData_[0] = 0;
		data_->indexData_[1] = 1;
		data_->indexData_[2] = 2;
		data_->indexData_[3] = 1;
		data_->indexData_[4] = 3;
		data_->indexData_[5] = 2;

		//PSO設定
		data_->psoDesc.RootSignatureID = RootSignatureManager::GetInstance()->GetOrCreateRootSignature(RootSigType::Particle);
		data_->psoDesc.VS_ID = ShaderManager::GetInstance()->CompileAndCasheShader(L"Resources/shader/Particle.VS.hlsl", L"vs_6_0");
		data_->psoDesc.PS_ID = ShaderManager::GetInstance()->CompileAndCasheShader(L"Resources/shader/Particle.PS.hlsl", L"ps_6_0");
		data_->psoDesc.InputLayoutID = InputLayoutType::Particle;
		data_->psoDesc.BlendMode = BlendModeType::Additive;
		data_->psoDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;	//Depthの書き込みを行わない

		UINT useIndex = SRVManager::GetInstance()->Allocate();

		SRVManager::GetInstance()->CreateSRVStructuredBuffer(useIndex, data_->instancingBuffer_.Get(), data_->kMaxParticleNum, sizeof(ParticleForGPU));
		break;

	case::ParticleType::Mesh:
		//CubeRendererがやってくれてるのでスキップ
		break;
	}
}

void ParticleRenderer::Update(const Matrix4x4& vpMatrix) {

}

void ParticleRenderer::Draw() {

}