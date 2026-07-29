#include "PCH.h"
#include "Outline.h"

void Outline::Initialize(ID3D12Device* device) {
	shadingModel_ = MyEngine::Rendering::ShadingModel::PostEffect_Outline;

	// リソースの初期化
	buffer_ = std::make_unique<ConstantBuffer<OutlineForGPU>>();
	buffer_->Initialize(device);

	// 初期パラメータの設定
	param_.color = { 0.0f, 0.0f, 0.0f, 1.0f }; // デフォルト：黒色不透明
	param_.edgeThreshold = 0.01f;               // デフォルト：エッジ感度
	param_.edgeThickness = 1.0f;                // デフォルト：エッジ太さ1倍
	param_.cameraNear = 0.1f;
	param_.cameraFar = 1000.0f;

	buffer_->Update(param_);
}

void Outline::ImGui() {
	bool changed = false;
	changed |= ImGui::ColorEdit4("Color##Outline", &param_.color.x);
	changed |= ImGui::DragFloat("Threshold##Outline", &param_.edgeThreshold, 0.001f, 0.0f, 1.0f, "%.4f");
	changed |= ImGui::DragFloat("Thickness##Outline", &param_.edgeThickness, 0.1f, 0.0f, 10.0f);
	ImGui::Text("CameraNear : %.3f", param_.cameraNear);
	ImGui::Text("CameraFar : %.3f", param_.cameraFar);

	// ImGuiで値が変わったら更新
	if (changed) {
		buffer_->Update(param_);
	}
}

D3D12_GPU_VIRTUAL_ADDRESS Outline::GetConstantBufferAddress() const {
	return buffer_->GetGPUVirtualAddress();
}
