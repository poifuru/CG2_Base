#include "RadialBlur.h"
#include "imgui.h"

void RadialBlur::Initialize(DxCommon* dxCommon) {
	BasePostEffect::Initialize(dxCommon);
	psoDesc_.PS_ID = ShaderManager::GetInstance()->CompileAndCacheShader(
		L"Resources/shader/RadialBlur.PS.hlsl", L"ps_6_0"
	);

	// リソースの初期化
	constantBuffer_ = dxCommon_->CreateBufferResource((sizeof(RadialBlurForGPU) + 255) & ~255);
	constantBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&cpuData_));

	cpuData_->center = { 0.5f, 0.5f };
	cpuData_->blurWidth = 0.01f;
	cpuData_->sampleCount = 10;
}

void RadialBlur::Draw(RenderTexture* renderTexture, CameraOrganizer* /*camera*/) {
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

void RadialBlur::ImGui() {
	ImGui::DragFloat2("center##RadialBlur", &cpuData_->center.x, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("blurWidth##RadialBlur", &cpuData_->blurWidth, 0.001f, 0.0f, 0.1f);
	ImGui::DragInt("sampleCount##RadialBlur", &cpuData_->sampleCount, 1.f, 1, 64);
}
