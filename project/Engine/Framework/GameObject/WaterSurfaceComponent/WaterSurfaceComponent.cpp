#include "PCH.h"
#include "WaterSurfaceComponent.h"
#include "DeltaTime.h"
#include "Model.h"

void WaterSurfaceComponent::Initialize() {
	if (auto* model = GetModel()) {
		model->SetShaders(MyEngine::Rendering::ShadingModel::WaterSurface);
	}
}

void WaterSurfaceComponent::Update() {
	// 時間を進める
	time_ += kDeltaTime;
	// マテリアルデータを更新する
	if (auto* model = GetModel()) {
		if (auto* material = model->GetMaterial()) {
			material->SetTime(time_); 
			material->SetRoughness(0.1f); // 水面なのでツルツルに
		}
	}
}

void WaterSurfaceComponent::Serialize(json& j) const {
	MeshRendererComponent::Serialize(j);
}

void WaterSurfaceComponent::Deserialize(const json& j) {
	MeshRendererComponent::Deserialize(j);
}
