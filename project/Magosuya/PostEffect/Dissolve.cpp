#include "Dissolve.h"
#include "TextureManager.h"
#include "imgui.h"

void Dissolve::Initialize(DxCommon* dxCommon) {
	BasePostEffect::Initialize(dxCommon);
	psoDesc_.RootSignatureID = RootSignatureManager::GetInstance()->GetOrCreateRootSignature(RootSigType::PostProcessTwoTextures);
	psoDesc_.PS_ID = ShaderManager::GetInstance()->CompileAndCacheShader(
		L"Resources/shader/Dissolve.PS.hlsl", L"ps_6_0"
	);

	// 定数バッファの初期化
	constantBuffer_ = dxCommon_->CreateBufferResource((sizeof(DissolveForGPU) + 255) & ~255);
	constantBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&cpuData_));

	cpuData_->edgeColor = { 1.0f, 0.5f, 0.0f, 1.0f }; // オレンジ色のエッジ
	cpuData_->threshold = 0.0f;                       // 閾値初期値
	cpuData_->edgeWidth = 0.02f;                      // エッジ幅初期値

	// マスクテクスチャのロード
	maskTexture_ = TextureManager::GetInstance()->LoadTexture("Resources/noise0.png", "dissolveMask");
}

void Dissolve::Draw(RenderTexture* renderTexture, CameraOrganizer* /*camera*/) {
	auto cmdList = dxCommon_->GetCommandList();

	RootSignatureManager::GetInstance()->SetRootSignature(psoDesc_.RootSignatureID);
	PSOManager::GetInstance()->SetPSO(psoDesc_);

	// プリミティブトポロジーを設定
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// CBVをセット (b0)
	cmdList->SetGraphicsRootConstantBufferView(0, constantBuffer_->GetGPUVirtualAddress());

	// SRVを設定 (t0: 元画面)
	SRVManager::GetInstance()->SetGraphicsRootDescriptorTable(1, renderTexture->GetSrvIndex());

	// SRVを設定 (t1: マスクテクスチャ)
	if (maskTexture_) {
		SRVManager::GetInstance()->SetGraphicsRootDescriptorTable(2, maskTexture_->descriptorIndex);
	}
	
	// 描画
	cmdList->DrawInstanced(3, 1, 0, 0);
}

void Dissolve::ImGui() {
	ImGui::DragFloat("threshold##Dissolve", &cpuData_->threshold, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("edgeWidth##Dissolve", &cpuData_->edgeWidth, 0.005f, 0.0f, 0.2f);
	ImGui::ColorEdit3("edgeColor##Dissolve", &cpuData_->edgeColor.x);
}
