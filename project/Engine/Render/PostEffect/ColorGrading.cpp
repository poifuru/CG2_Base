#include "PCH.h"
//#include "ColorGrading.h"
//#include "imgui.h"
//
//void ColorGrading::Initialize(DxCommon* dxCommon) {
//	BasePostEffect::Initialize(dxCommon);
//	psoDesc_.PS_ID = ShaderManager::GetInstance()->CompileAndCacheShader(
//		L"Resources/shader/ColorGrading.PS.hlsl", L"ps_6_0"
//	);
//
//	// リソースの初期化
//	constantBuffer_ = dxCommon_->CreateBufferResource(sizeof(ColorGradingForGPU));
//	constantBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&cpuData_));
//
//	cpuData_->intensity = 1.0f;
//	cpuData_->sepiaAmount = 0.0f;
//	cpuData_->tintColor = { 1.0f, 0.95f, 0.82f };
//}
//
//void ColorGrading::Draw(RenderTexture* renderTexture, CameraOrganizer* /*camera*/) {
//	auto cmdList = dxCommon_->GetCommandList();
//
//	RootSignatureManager::GetInstance()->SetRootSignature(psoDesc_.RootSignatureID);
//	PSOManager::GetInstance()->SetPSO(psoDesc_);
//
//	// プリミティブトポロジーを設定
//	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	// CBVをセット
//	cmdList->SetGraphicsRootConstantBufferView(0, constantBuffer_->GetGPUVirtualAddress());
//
//	// SRVを設定 (t0)
//	SRVManager::GetInstance()->SetGraphicsRootDescriptorTable(1, renderTexture->GetSrvIndex());
//
//	// 描画
//	cmdList->DrawInstanced(3, 1, 0, 0);
//}
//
//void ColorGrading::ImGui() {
//	ImGui::DragFloat("intensity##ColorGrading", &cpuData_->intensity, 0.01f, 0.0f, 1.0f);
//	ImGui::DragFloat("sepiaAmount##ColorGrading", &cpuData_->sepiaAmount, 0.01f, 0.0f, 1.0f);
//	ImGui::DragFloat3("tintColor##ColorGrading", &cpuData_->tintColor.x, 0.01f, 0.0f, 1.0f);
//}