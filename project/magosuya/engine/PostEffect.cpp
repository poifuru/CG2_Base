#include "PostEffect.h"
#include "DxCommon.h"
#include "RenderTexture.h"
#include "ShaderManager.h"
#include "RootSignatureManager.h"
#include "PSOManager.h"

void PostEffect::Initialize(DxCommon* dxCommon) {
	dxCommon_ = dxCommon;

	// シェーダーのコンパイル
	uint32_t vsID = ShaderManager::GetInstance()->CompileAndCacheShader(L"Resources/shader/CopyImage.VS.hlsl", L"vs_6_0");
	uint32_t psID = ShaderManager::GetInstance()->CompileAndCacheShader(L"Resources/shader/CopyImage.PS.hlsl", L"ps_6_0");

	// RootSignatureの取得
	uint32_t rootSigID = RootSignatureManager::GetInstance()->GetOrCreateRootSignature(RootSigType::PostProcess);
	rootSignature_ = RootSignatureManager::GetInstance()->GetRootSignature(rootSigID);

	// PSOの設定
	PSODescriptor psoDesc{};
	psoDesc.RootSignatureID = rootSigID;
	psoDesc.VS_ID = vsID;
	psoDesc.PS_ID = psID;
	psoDesc.InputLayoutID = InputLayoutType::PostProcess;
	psoDesc.BlendMode = BlendModeType::Opaque;
	
	// ラスタライザステート
	psoDesc.CullMode = D3D12_CULL_MODE_NONE; // カリングなし
	psoDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// デプスステンシルステート (2D描画なのでデプス無効)
	psoDesc.DepthEnable = FALSE;
	psoDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

	// その他の設定
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	psoDesc.NumRenderTargets = 1;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	psoDesc.SampleCount = 1;

	pso_ = PSOManager::GetInstance()->GetOrCreatePSO(psoDesc);
}

void PostEffect::Draw(RenderTexture* renderTexture) {
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	// RootSignatureを設定
	commandList->SetGraphicsRootSignature(rootSignature_);
	
	// PSOを設定
	commandList->SetPipelineState(pso_);

	// プリミティブトポロジーを設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// SRVを設定 (t0)
	SRVManager::GetInstance()->SetGraphicsRootDescriptorTable(0, renderTexture->GetSrvIndex());

	// 頂点バッファ無しで3頂点描画（SV_VertexIDを利用してフルスクリーントライアングルを生成）
	commandList->DrawInstanced(3, 1, 0, 0);
}
