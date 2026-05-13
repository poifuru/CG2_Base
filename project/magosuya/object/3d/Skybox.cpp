#include "Skybox.h"
#include "TextureManager.h"
#include "MathFunction.h"

const uint32_t kVertexNum = 8;
const uint32_t kIndexNum = 36;

Skybox::Skybox(DxCommon* dxCommon) {
	dxCommon_ = dxCommon;
	device_ = dxCommon->GetDevice();
	commandList_ = dxCommon->GetCommandList();
}

void Skybox::Initialize(std::string filePath, std::string tag) {
	TextureManager::GetInstance()->LoadTexture(filePath, tag);
	tag_ = tag;

	// === リソース作成 === //
	// 頂点データ
	vertexBuffer_ = dxCommon_->CreateBufferResource(sizeof(SkyboxVertex) * kVertexNum);
	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	// バッファビュー
	vbv_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	vbv_.SizeInBytes = UINT(sizeof(SkyboxVertex) * kVertexNum);
	vbv_.StrideInBytes = sizeof(SkyboxVertex);

	// 実際の頂点データ
	vertexData_[0].position = { -1.0f, 1.0f, 1.0f, 1.0f };
	vertexData_[1].position = { 1.0f, 1.0f, 1.0f, 1.0f };
	vertexData_[2].position = { -1.0f, -1.0f, 1.0f, 1.0f };
	vertexData_[3].position = { 1.0f, -1.0f, 1.0f, 1.0f };
	vertexData_[4].position = { -1.0f, 1.0f, -1.0f, 1.0f };
	vertexData_[5].position = { 1.0f, 1.0f, -1.0f, 1.0f };
	vertexData_[6].position = { -1.0f, -1.0f, -1.0f, 1.0f };
	vertexData_[7].position = { 1.0f, -1.0f, -1.0f, 1.0f };

	// インデックスデータ
	indexBuffer_ = dxCommon_->CreateBufferResource(sizeof(uint32_t) * kIndexNum);
	indexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));
	// バッファビュー
	ibv_.BufferLocation = indexBuffer_->GetGPUVirtualAddress();
	ibv_.SizeInBytes = UINT(sizeof(uint32_t) * kIndexNum);
	ibv_.Format = DXGI_FORMAT_R32_UINT;

	// 実際のインデックスデータ
	indexData_[0] = 0;		// 前
	indexData_[1] = 1;
	indexData_[2] = 2;
	indexData_[3] = 1;
	indexData_[4] = 3;
	indexData_[5] = 2;

	indexData_[6] = 5;		// 後ろ
	indexData_[7] = 4;
	indexData_[8] = 7;
	indexData_[9] = 4;
	indexData_[10] = 6;
	indexData_[11] = 7;

	indexData_[12] = 1;		// 右
	indexData_[13] = 5;
	indexData_[14] = 3;
	indexData_[15] = 5;
	indexData_[16] = 7;
	indexData_[17] = 3;

	indexData_[18] = 4;		// 左
	indexData_[19] = 0;
	indexData_[20] = 6;
	indexData_[21] = 0;
	indexData_[22] = 2;
	indexData_[23] = 6;

	indexData_[24] = 4;		// 上
	indexData_[25] = 5;
	indexData_[26] = 0;
	indexData_[27] = 5;
	indexData_[28] = 1;
	indexData_[29] = 0;

	indexData_[30] = 2;		// 下
	indexData_[31] = 3;
	indexData_[32] = 6;
	indexData_[33] = 3;
	indexData_[34] = 7;
	indexData_[35] = 6;

	// 行列データ
	matrixBuffer_ = dxCommon_->CreateBufferResource(sizeof(TransformationMatrix));
	matrixBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&matrixData_));
	matrixData_->World = Math::MakeIdentity4x4();
	matrixData_->WVP = Math::MakeIdentity4x4();
	matrixData_->WorldInverseTranspose = Math::MakeIdentity4x4();

	// マテリアルデータ
	materialBuffer_ = dxCommon_->CreateBufferResource(sizeof(Material));
	materialBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData_->enableLighting = LightReflectionModel::None;
	materialData_->uvTransform = Math::MakeIdentity4x4();
	materialData_->roughness = 0.0f;
	materialData_->metallic = 0.0f;

	// PSOの設定
	desc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	desc_.RootSignatureID = RootSignatureManager::GetInstance()->GetOrCreateRootSignature(RootSigType::Skybox);
	desc_.VS_ID = ShaderManager::GetInstance()->CompileAndCacheShader(L"Resources/shader/Skybox.VS.hlsl", L"vs_6_0");
	desc_.PS_ID = ShaderManager::GetInstance()->CompileAndCacheShader(L"Resources/shader/Skybox.PS.hlsl", L"ps_6_0");
	desc_.InputLayoutID = InputLayoutType::Skybox;
	desc_.BlendMode = BlendModeType::Opaque;
}

void Skybox::Update(CameraData* data) {
	EulerTransform transform = { { 5000.0f, 5000.0f, 5000.0f}, {}, {} };
	matrixData_->World = Math::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

	if(data) {
		matrixData_->WVP = Math::Multiply(matrixData_->World, data->vp);
	}
}

void Skybox::Draw() {
	RootSignatureManager::GetInstance()->SetRootSignature(desc_.RootSignatureID);
	PSOManager::GetInstance()->SetPSO(desc_);
	// どんな形状で描画するのか
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// 頂点バッファをセットする
	commandList_->IASetVertexBuffers(0, 1, &vbv_);
	// インデックスバッファをセットする
	commandList_->IASetIndexBuffer(&ibv_);
	// 行列バッファ
	commandList_->SetGraphicsRootConstantBufferView(0, matrixBuffer_->GetGPUVirtualAddress());
	// マテリアルバッファ
	commandList_->SetGraphicsRootConstantBufferView(1, materialBuffer_->GetGPUVirtualAddress());
	// テクスチャハンドル
	commandList_->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetTextureHandle(tag_));

	// 実際の描画命令(インデックス描画)
	commandList_->DrawIndexedInstanced(36, 1, 0, 0, 0);
}
