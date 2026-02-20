#include "CubeRenderer.h"
#include "MathFunction.h"
#include "MaxMeshNum.h"
#include "DxCommon.h"

//インデックス描画用のインデックス数
const uint32_t indexNum = 3 * 2 * 6;

CubeRenderer::~CubeRenderer () {

}

void CubeRenderer::Initialize (DxCommon* dxCommon) {
	dxCommon_ = dxCommon;
	device_ = dxCommon->GetDevice ();
	commandList_ = dxCommon->GetCommandList ();
	srvManager_ = SRVManager::GetInstance();

	cubeBuffer_ = std::make_unique<CubeVertexData> ();

	//頂点バッファー作成とマッピング
	cubeBuffer_->vertexBuffer = dxCommon_->CreateBufferResource (sizeof (CubeVertexPositionColor) * VertexNum::Cube * MaxMeshNum::Cube);
	cubeBuffer_->vertexBuffer->Map (0, nullptr, reinterpret_cast<void**>(&vertexData_));
	cubeBuffer_->vbView.BufferLocation = cubeBuffer_->vertexBuffer->GetGPUVirtualAddress ();
	cubeBuffer_->vbView.SizeInBytes = sizeof (CubeVertexPositionColor) * VertexNum::Cube * MaxMeshNum::Cube;
	cubeBuffer_->vbView.StrideInBytes = sizeof (CubeVertexPositionColor);

	//インデックスバッファー作成とマッピング
	cubeBuffer_->indexBuffer = dxCommon_->CreateBufferResource (sizeof (uint32_t) * indexNum * MaxMeshNum::Cube);
	cubeBuffer_->indexBuffer->Map (0, nullptr, reinterpret_cast<void**>(&indexData_));
	cubeBuffer_->ibView.BufferLocation = cubeBuffer_->indexBuffer->GetGPUVirtualAddress ();
	cubeBuffer_->ibView.SizeInBytes = sizeof (uint32_t) * indexNum * MaxMeshNum::Cube;
	cubeBuffer_->ibView.Format = DXGI_FORMAT_R32_UINT;

	//行列バッファー作成とマッピング(頂点2つにつき1つ)
	instancingBuffer_ = dxCommon_->CreateBufferResource (sizeof (CubeForGPU) * MaxMeshNum::Cube);
	instancingBuffer_->Map (0, nullptr, reinterpret_cast<void**>(&instancingData_));
	for (uint32_t i = 0; i < MaxMeshNum::Cube; ++i) {
		instancingData_[i].World = Math::MakeIdentity4x4 ();
		instancingData_[i].WVP = Math::MakeIdentity4x4 ();
	}

	//使うインデックスを確保
	instancingIndex_ = srvManager_->Allocate();
	vertexIndex_ = srvManager_->Allocate();

	//インスタンシング用のSRV作成
	srvManager_->CreateSRVStructuredBuffer(instancingIndex_, instancingBuffer_.Get(), MaxMeshNum::Cube, sizeof(CubeForGPU));

	//頂点バッファ用のSRV作成
	srvManager_->CreateSRVStructuredBuffer(vertexIndex_ + 1, cubeBuffer_->vertexBuffer.Get(), VertexNum::Cube, sizeof(CubeVertexData));

	//PSOの設定
	desc_.RootSignatureID = RootSignatureManager::GetInstance ()->GetOrCreateRootSignature (RootSigType::CubeMesh);
	desc_.VS_ID = ShaderManager::GetInstance ()->CompileAndCacheShader (L"Resources/shader/Cube.VS.hlsl", L"vs_6_0");
	desc_.PS_ID = ShaderManager::GetInstance ()->CompileAndCacheShader (L"Resources/shader/Cube.PS.hlsl", L"ps_6_0");
	desc_.InputLayoutID = InputLayoutType::CubeMesh;
	desc_.BlendMode = BlendModeType::Additive;
	desc_.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;	//三角形で描画
	PSOManager::GetInstance ()->GetOrCreatePSO (desc_);
}

void CubeRenderer::UpdateVertexData (const CubeData* data) {
	//currentCubeNumが最大数を超えていないか
	if (currentCubeCount_ >= MaxMeshNum::Cube) {
		//超えてたら早期リターン
		return;
	}

	// 現在の描画位置のオフセット (キューブ1個 = 8頂点)
	uint32_t offset = currentCubeCount_ * VertexNum::Cube;

#pragma region 頂点の情報をコピー
	//オフセットを使ってライン描画に必要な分だけバッファーにコピー
	vertexData_[offset + 0].position = { data->transform.translate.x - data->size, data->transform.translate.y - data->size, data->transform.translate.z - data->size };
	vertexData_[offset + 0].color = data->color[0];

	vertexData_[offset + 1].position = { data->transform.translate.x - data->size, data->transform.translate.y - data->size, data->transform.translate.z + data->size };
	vertexData_[offset + 1].color = data->color[1];

	vertexData_[offset + 2].position = { data->transform.translate.x + data->size, data->transform.translate.y - data->size, data->transform.translate.z + data->size };
	vertexData_[offset + 2].color = data->color[2];

	vertexData_[offset + 3].position = { data->transform.translate.x + data->size, data->transform.translate.y - data->size, data->transform.translate.z - data->size };
	vertexData_[offset + 3].color = data->color[3];

	vertexData_[offset + 4].position = { data->transform.translate.x - data->size, data->transform.translate.y + data->size, data->transform.translate.z - data->size };
	vertexData_[offset + 4].color = data->color[4];

	vertexData_[offset + 5].position = { data->transform.translate.x - data->size, data->transform.translate.y + data->size, data->transform.translate.z + data->size };
	vertexData_[offset + 5].color = data->color[5];

	vertexData_[offset + 6].position = { data->transform.translate.x + data->size, data->transform.translate.y + data->size, data->transform.translate.z + data->size };
	vertexData_[offset + 6].color = data->color[6];

	vertexData_[offset + 7].position = { data->transform.translate.x + data->size, data->transform.translate.y + data->size, data->transform.translate.z - data->size };
	vertexData_[offset + 7].color = data->color[7];
#pragma endregion

	//indexBufferの位置のオフセット
	uint32_t indexOffset = currentCubeCount_ * indexNum;

#pragma region インデックスの配置
	//どのインデックスにどの頂点を入れるか
	//底面(0, 1, 2, 3)
	indexData_[indexOffset + 0] = 0;
	indexData_[indexOffset + 1] = 3;
	indexData_[indexOffset + 2] = 1;

	indexData_[indexOffset + 3] = 2;
	indexData_[indexOffset + 4] = 1;
	indexData_[indexOffset + 5] = 3;

	//上面(4, 5, 6, 7)
	indexData_[indexOffset + 6] = 4;
	indexData_[indexOffset + 7] = 7;
	indexData_[indexOffset + 8] = 5;

	indexData_[indexOffset + 9] = 6;
	indexData_[indexOffset + 10] = 5;
	indexData_[indexOffset + 11] = 7;

	//側面1(0, 4, 7 ,3)
	indexData_[indexOffset + 12] = 0;
	indexData_[indexOffset + 13] = 3;
	indexData_[indexOffset + 14] = 4;

	indexData_[indexOffset + 15] = 7;
	indexData_[indexOffset + 16] = 4;
	indexData_[indexOffset + 17] = 3;

	//側面2(1, 5, 4, 0)
	indexData_[indexOffset + 18] = 1;
	indexData_[indexOffset + 19] = 0;
	indexData_[indexOffset + 20] = 5;

	indexData_[indexOffset + 21] = 4;
	indexData_[indexOffset + 22] = 5;
	indexData_[indexOffset + 23] = 0;

	//側面3(2, 6, 5, 1)
	indexData_[indexOffset + 24] = 2;
	indexData_[indexOffset + 25] = 1;
	indexData_[indexOffset + 26] = 6;

	indexData_[indexOffset + 27] = 5;
	indexData_[indexOffset + 28] = 6;
	indexData_[indexOffset + 29] = 1;

	//側面4(3, 7, 6, 2)
	indexData_[indexOffset + 30] = 3;
	indexData_[indexOffset + 31] = 2;
	indexData_[indexOffset + 32] = 7;

	indexData_[indexOffset + 33] = 6;
	indexData_[indexOffset + 34] = 7;
	indexData_[indexOffset + 35] = 2;
#pragma endregion

	//線の数をインクリメント
	currentCubeCount_++;
}

void CubeRenderer::TransferData (const CubeForGPU& data) {
	//描画要求数が0か最大数を超えてるなら
	if (currentCubeCount_ == 0 || currentCubeCount_ > MaxMeshNum::Cube) {
		//早期リターン
		return;
	}

	// 書き込むインスタンス配列のインデックス
	uint32_t instanceIndex = currentCubeCount_ - 1;

	instancingData_[instanceIndex].World = data.World;
	instancingData_[instanceIndex].WVP = data.WVP;
}

void CubeRenderer::Draw () {
	//RootSignatureとPSOをセット
	RootSignatureManager::GetInstance ()->SetRootSignature (desc_.RootSignatureID);
	PSOManager::GetInstance ()->SetPSO (desc_);
	commandList_->IASetPrimitiveTopology (D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);		//三角形で描画	
	commandList_->IASetIndexBuffer (&cubeBuffer_->ibView);		//IBVを設定
	commandList_->SetGraphicsRootDescriptorTable (1, srvManager_->GetGPUDescriptorHandle(instancingIndex_));

	if (currentCubeCount_ > 0) {
		// 描画
		commandList_->DrawIndexedInstanced (indexNum, currentCubeCount_, 0, 0, 0); // 描画要求があった頂点数だけ描画
	}

	//次フレームのためにカウントをリセット
	ResetCurrentCubeCount ();
}