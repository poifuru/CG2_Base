#include "PCH.h"
#include "WaterSurfaceComponent.h"
#include "DeltaTime.h"
#include "Model.h"
#include "BaseScene.h"
#include "GameObject.h"
#include "GraphicsDevice.h"

void WaterSurfaceComponent::Initialize() {
	MeshRendererComponent::Initialize();

	if (auto* model = GetModel()) {
		model->SetShaders(MyEngine::Rendering::ShadingModel::WaterSurface);
	}

	auto* device = GetGameObject()->GetContext()->graphicsDevice->GetDevice(); // メンバにあればそこから取得
	waterSurfaceBuffer_.Initialize(device);

	if (auto* model = GetModel()) {
		model->SetShaders(MyEngine::Rendering::ShadingModel::WaterSurface);
	}
}

void WaterSurfaceComponent::Update() {
	MeshRendererComponent::Update();

	// 時間を進める
	time_ += kDeltaTime;
	// マテリアルデータを更新する
	if (auto* model = GetModel()) {
		if (auto* material = model->GetMaterial()) {
			material->SetTime(time_); 
			material->SetRoughness(0.1f); // 水面なのでツルツルに
		}
	}

	// パラメータをGPUバッファにアップデート
	WaterSurfaceInfo params{};
	params.amplitude = amplitude_;
	params.frequency = frequency_;
	params.steepness = steepness_;
	params.time = time_;
	params.direction = direction_;
	waterSurfaceBuffer_.Update(params);

	if (auto* model = GetModel()) {
		if (auto* material = model->GetMaterial()) {
			material->SetRoughness(0.1f);
		}
	}
}

void WaterSurfaceComponent::ImGui() {
	MeshRendererComponent::ImGui();

	ImGui::Separator();
	ImGui::Text("Gerstner Wave Parameters");

	// 各パラメータのスライダーを設置
	ImGui::DragFloat("Amplitude (A)", &amplitude_, 0.01f, 0.0f, 10.0f);
	ImGui::DragFloat("Frequency (w)", &frequency_, 0.01f, 0.0f, 5.0f);
	ImGui::DragFloat("Steepness (Q)", &steepness_, 0.01f, 0.0f, 1.0f); // Qは0〜1

	// 方向ベクトルの編集
	if (ImGui::DragFloat2("Direction (D)", &direction_.x, 0.01f, -1.0f, 1.0f)) {
		// 方向は正規化しておく
		float len = std::sqrt(direction_.x * direction_.x + direction_.y * direction_.y);
		if (len > 0.001f) {
			direction_.x /= len;
			direction_.y /= len;
		}
	}
}

void WaterSurfaceComponent::Serialize(json& j) const {
	MeshRendererComponent::Serialize(j);
	j["type"] = "WaterSurfaceComponent";

	j["wave"]["amplitude"] = amplitude_;
	j["wave"]["frequency"] = frequency_;
	j["wave"]["steepness"] = steepness_;
	j["wave"]["direction"] = { direction_.x, direction_.y };
}

void WaterSurfaceComponent::Deserialize(const json& j) {
	MeshRendererComponent::Deserialize(j);

	if (j.contains("wave")) {
		const auto& wJ = j["wave"];
		if (wJ.contains("amplitude")) amplitude_ = wJ["amplitude"];
		if (wJ.contains("frequency")) frequency_ = wJ["frequency"];
		if (wJ.contains("steepness")) steepness_ = wJ["steepness"];
		if (wJ.contains("direction")) {
			direction_.x = wJ["direction"][0];
			direction_.y = wJ["direction"][1];
		}
	}

	// デシリアライズでデータが読み込まれた「後」に、強制的にシェーダーを水面用に上書きする！
	if (auto* model = GetModel()) {
		model->SetShaders(MyEngine::Rendering::ShadingModel::WaterSurface);
	}
}