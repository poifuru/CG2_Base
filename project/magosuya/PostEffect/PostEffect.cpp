#include "PostEffect.h"
#include "DxCommon.h"
#include "RenderTexture.h"
#include "ShaderManager.h"
#include "RootSignatureManager.h"
#include "PSOManager.h"
#include "SRVManager.h"
#include "imgui.h"

void PostEffect::Initialize(DxCommon* dxCommon) {
	dxCommon_ = dxCommon;

	// シェーダーのコンパイル
	uint32_t vsID = ShaderManager::GetInstance()->CompileAndCacheShader(L"Resources/shader/Fullscreen.VS.hlsl", L"vs_6_0");
	uint32_t psID = ShaderManager::GetInstance()->CompileAndCacheShader(L"Resources/shader/GrayScale.PS.hlsl", L"ps_6_0");

	// RootSignatureの取得
	uint32_t rootSigID = RootSignatureManager::GetInstance()->GetOrCreateRootSignature(RootSigType::PostProcess);
	rootSignature_ = RootSignatureManager::GetInstance()->GetRootSignature(rootSigID);

	// PSOの設定
	psoDesc_riptor psoDesc_{};
	psoDesc_.RootSignatureID = rootSigID;
	psoDesc_.VS_ID = vsID;
	psoDesc_.PS_ID = psID;
	psoDesc_.InputLayoutID = InputLayoutType::PostProcess;
	psoDesc_.BlendMode = BlendModeType::Opaque;
	
	// ラスタライザステート
	psoDesc_.CullMode = D3D12_CULL_MODE_NONE; // カリングなし
	psoDesc_.FillMode = D3D12_FILL_MODE_SOLID;

	// デプスステンシルステート (2D描画なのでデプス無効)
	psoDesc_.DepthEnable = FALSE;
	psoDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

	// その他の設定
	psoDesc_.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc_.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	psoDesc_.NumRenderTargets = 1;
	psoDesc_.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	psoDesc_.SampleCount = 1;

	pso_ = PSOManager::GetInstance()->GetOrCreatePSO(psoDesc_);

	postProcessResource_ = dxCommon_->CreateBufferResource(sizeof(PostProcessData));
	postProcessResource_->Map(0, nullptr, reinterpret_cast<void**>(&postProcessData_));
	postProcessData_->intensity = 0.0f;
	postProcessData_->time = 0.0f;
}

void PostEffect::Draw(RenderTexture* renderTexture) {
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	// RootSignatureを設定
	commandList->SetGraphicsRootSignature(rootSignature_);
	
	// PSOを設定
	commandList->SetPipelineState(pso_);

	// プリミティブトポロジーを設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// CBVをセット
	commandList->SetGraphicsRootConstantBufferView(0, postProcessResource_->GetGPUVirtualAddress());

	// SRVを設定 (t0)
	SRVManager::GetInstance()->SetGraphicsRootDescriptorTable(1, renderTexture->GetSrvIndex());

	// 頂点バッファ無しで3頂点描画（SV_VertexIDを利用してフルスクリーントライアングルを生成）
	commandList->DrawInstanced(3, 1, 0, 0);
}

void PostEffect::Imgui() {
#ifdef USEIMGUI
	ImGui::Begin("PostEffect");
	ImGui::DragFloat("intensity", &postProcessData_->intensity, 0.001f);
	ImGui::DragFloat("time", &postProcessData_->time, 0.001f);
	ImGui::End();
#endif
}