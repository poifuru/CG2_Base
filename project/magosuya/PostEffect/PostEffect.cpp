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

	// PSOの設定
	psoDesc_.VS_ID = ShaderManager::GetInstance()->CompileAndCacheShader(L"Resources/shader/Fullscreen.VS.hlsl", L"vs_6_0");
	psoDesc_.PS_ID = ShaderManager::GetInstance()->CompileAndCacheShader(L"Resources/shader/GrayScale.PS.hlsl", L"ps_6_0");
	psoDesc_.RootSignatureID = RootSignatureManager::GetInstance()->GetOrCreateRootSignature(RootSigType::PostProcess);
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

	postProcessResource_ = dxCommon_->CreateBufferResource(sizeof(PostProcessData));
	postProcessResource_->Map(0, nullptr, reinterpret_cast<void**>(&postProcessData_));
	postProcessData_->intensity = 0.0f;
	postProcessData_->time = 0.0f;
}

void PostEffect::Draw(RenderTexture* renderTexture) {
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	RootSignatureManager::GetInstance()->SetRootSignature(psoDesc_.RootSignatureID);
	PSOManager::GetInstance()->SetPSO(psoDesc_);

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