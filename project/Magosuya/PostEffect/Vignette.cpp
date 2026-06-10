#include "Vignette.h"
#include "imgui.h"
#include "WindowsAPI.h"

void Vignette::Initialize(DxCommon* dxCommon) {
	BasePostEffect::Initialize(dxCommon);
	psoDesc_.PS_ID = ShaderManager::GetInstance()->CompileAndCacheShader(
		L"Resources/shader/Vignette.PS.hlsl", L"ps_6_0"
	);

	// リソースの初期化
	constantBuffer_ = dxCommon_->CreateBufferResource(sizeof(VignetteForGPU));
	constantBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&cpuData_));

	cpuData_->center = { 0.5f, 0.5f };
	cpuData_->innerRadius = 0.0f;
	cpuData_->outerRadius = 0.0f;
	cpuData_->vignetteColor = { 0.0f, 0.0f, 0.0f };
	cpuData_->intensity = 1.0f;
	cpuData_->aspectRatio =
		static_cast<float>(WindowsAPI::GetInstance()->GetWindowWidth() / WindowsAPI::GetInstance()->GetWindowHeight());
}

void Vignette::Draw(RenderTexture* renderTexture) {
	auto cmdList = dxCommon_->GetCommandList();

	RootSignatureManager::GetInstance()->SetRootSignature(psoDesc_.RootSignatureID);
	PSOManager::GetInstance()->SetPSO(psoDesc_);

	// プリミティブトポロジーを設定
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// CBVをセット
	cmdList->SetGraphicsRootConstantBufferView(0, constantBuffer_->GetGPUVirtualAddress());

	// SRVを設定 (t0)
	SRVManager::GetInstance()->SetGraphicsRootDescriptorTable(1, renderTexture->GetSrvIndex());

	// 描画
	cmdList->DrawInstanced(3, 1, 0, 0);
}

void Vignette::ImGui() {
	ImGui::Checkbox("isActive##Vignette", &isActive_);
	ImGui::DragFloat2("center##Vignette", &cpuData_->center.x, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("innerRadius##Vignette", &cpuData_->innerRadius, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("outerRadius##Vignette", &cpuData_->outerRadius, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat3("vignetteColor##Vignette", &cpuData_->vignetteColor.x, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("intensity##Vignette", &cpuData_->intensity, 0.01f, 0.0f, 1.0f);
	ImGui::Text("aspectRatio##Vignette", &cpuData_->aspectRatio);
}