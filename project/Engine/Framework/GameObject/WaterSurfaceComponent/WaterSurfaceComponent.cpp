#include "PCH.h"
#include "WaterSurfaceComponent.h"
#include "DeltaTime.h"
#include "Model.h"
#include "MeshData.h"
#include "BaseScene.h"
#include "GameObject.h"
#include "GraphicsDevice.h"
#include "ComponentType.h"

void WaterSurfaceComponent::Initialize() {
	if(isInitialized_) return;

	MeshRendererComponent::Initialize();

	if (auto* model = GetModel()) {
		model->SetShaders(MyEngine::Rendering::ShadingModel::WaterSurface);
	}

	auto* device = GetGameObject()->GetContext()->graphicsDevice->GetDevice(); // メンバにあればそこから取得
	waterSurfaceBuffer_.Initialize(device);

	if (auto* model = GetModel()) {
		model->SetShaders(MyEngine::Rendering::ShadingModel::WaterSurface);
		model->SetBlendMode(MyEngine::Rendering::BlendModeType::Alpha);
	}

	// 波のパラメータ初期化
	// 波1: 大きくゆっくり進む波
	waves_[0].amplitude = 0.3f;
	waves_[0].frequency = 0.4f;
	waves_[0].steepness = 0.5f;
	waves_[0].direction = Vector2(1.0f, 1.0f);
	// 方向ベクトルの正規化
	{
		float len = std::sqrt(waves_[0].direction.x * waves_[0].direction.x + waves_[0].direction.y * waves_[0].direction.y);
		if (len > 0.001f) {
			waves_[0].direction.x /= len;
			waves_[0].direction.y /= len;
		}
	}
	// 波2: 細かくて少し速いクロスする波
	waves_[1].amplitude = 0.1f;
	waves_[1].frequency = 1.2f;
	waves_[1].steepness = 0.3f;
	waves_[1].direction = Vector2(-1.0f, 1.0f);
	{
		float len = std::sqrt(waves_[1].direction.x * waves_[1].direction.x + waves_[1].direction.y * waves_[1].direction.y);
		if (len > 0.001f) {
			waves_[1].direction.x /= len;
			waves_[1].direction.y /= len;
		}
	}
	// 残りの波は初期値0（フラット）にしておく
	for (int i = 2; i < 4; ++i) {
		waves_[i].amplitude = 0.0f;
		waves_[i].frequency = 1.0f;
		waves_[i].steepness = 0.0f;
		waves_[i].direction = Vector2(1.0f, 0.0f);
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

	// GPU送信用パラメータの構築
    WaterSurfaceForGPU params{};
    params.time = time_;
    params.numActiveWaves = numActiveWaves_;
    for (int i = 0; i < 4; ++i) {
        params.waves[i] = waves_[i];
    }
	params.nearFadeDistance = nearFadeDistance_;
	params.farFadeDistance = farFadeDistance_;

	// シーン内のオブジェクトから BoatWakeComponent を探す
	auto* wakeComp = GetGameObject()->FindComponentInScene<BoatWakeComponent>();

	if (wakeComp) {
		// 見つかった BoatWakeComponent から CS 波紋テクスチャのインデックスを取得！
		params.rippleTextureIndex = wakeComp->GetRippleTextureIndex();
	}

	params.waterMin = GetWaterMin();
	params.waterSize = GetWaterSize();

    // 定数バッファを更新
    waterSurfaceBuffer_.Update(params);
}

void WaterSurfaceComponent::ImGui() {
	MeshRendererComponent::ImGui();

	ImGui::Separator();
	ImGui::Text("Gerstner Wave System");
	// 有効にする波の数 (1〜4)
	ImGui::SliderInt("Active Waves Count", &numActiveWaves_, 1, 4);
	// 各波のパラメータを編集
	for (int i = 0; i < numActiveWaves_; ++i) {
		ImGui::PushID(i); // IDの衝突を防ぐため、波のインデックスをPush

		char label[64];
		sprintf_s(label, "Wave %d", i + 1);
		if (ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::DragFloat("Amplitude (A)", &waves_[i].amplitude, 0.01f, 0.0f, 5.0f);
			ImGui::DragFloat("Frequency (w)", &waves_[i].frequency, 0.01f, 0.0f, 5.0f);
			ImGui::DragFloat("Steepness (Q)", &waves_[i].steepness, 0.01f, 0.0f, 1.0f);

			if (ImGui::DragFloat2("Direction (D)", &waves_[i].direction.x, 0.01f, -1.0f, 1.0f)) {
				// 方向ベクトルを変更したら必ず正規化する
				float len = std::sqrt(waves_[i].direction.x * waves_[i].direction.x + waves_[i].direction.y * waves_[i].direction.y);
				if (len > 0.001f) {
					waves_[i].direction.x /= len;
					waves_[i].direction.y /= len;
				}
			}
		}

		ImGui::PopID();
	}

	ImGui::Separator();
	ImGui::Text("Water Transparency Fade");
	ImGui::DragFloat("Near Fade Distance", &nearFadeDistance_, 1.0f, 0.0f, 1000.0f);
	ImGui::DragFloat("Far Fade Distance", &farFadeDistance_, 1.0f, 0.0f, 1000.0f);
}

void WaterSurfaceComponent::Serialize(json& j) const {
	MeshRendererComponent::Serialize(j);
	j["type"] = "WaterSurfaceComponent";
	j["numActiveWaves"] = numActiveWaves_;

	json wavesJ = json::array();
	for (int i = 0; i < 4; ++i) {
		json w;
		w["amplitude"] = waves_[i].amplitude;
		w["frequency"] = waves_[i].frequency;
		w["steepness"] = waves_[i].steepness;
		w["direction"] = { waves_[i].direction.x, waves_[i].direction.y };
		wavesJ.push_back(w);
	}
	j["waves"] = wavesJ;

	j["nearFadeDistance"] = nearFadeDistance_;
	j["farFadeDistance"] = farFadeDistance_;
}

void WaterSurfaceComponent::Deserialize(const json& j) {
	MeshRendererComponent::Deserialize(j);

	// 定数バッファを確実に初期化する 
	auto* device = GetGameObject()->GetContext()->graphicsDevice->GetDevice();
	waterSurfaceBuffer_.Initialize(device);

	if (j.contains("numActiveWaves")) {
		numActiveWaves_ = j["numActiveWaves"];
	}
	if (j.contains("waves") && j["waves"].is_array()) {
		const auto& wavesJ = j["waves"];
		int count = std::min(4, (int)wavesJ.size());
		for (int i = 0; i < count; ++i) {
			const auto& wJ = wavesJ[i];
			if (wJ.contains("amplitude")) waves_[i].amplitude = wJ["amplitude"];
			if (wJ.contains("frequency")) waves_[i].frequency = wJ["frequency"];
			if (wJ.contains("steepness")) waves_[i].steepness = wJ["steepness"];
			if (wJ.contains("direction")) {
				waves_[i].direction.x = wJ["direction"][0];
				waves_[i].direction.y = wJ["direction"][1];
			}
		}
	}
	if (j.contains("nearFadeDistance")) {
		nearFadeDistance_ = j["nearFadeDistance"];
	}
	if (j.contains("farFadeDistance")) {
		farFadeDistance_ = j["farFadeDistance"];
	}

	// デシリアライズでデータが読み込まれた後に、強制的にシェーダーを水面用に上書き
	if (auto* model = GetModel()) {
		model->SetShaders(MyEngine::Rendering::ShadingModel::WaterSurface);
		model->SetBlendMode(MyEngine::Rendering::BlendModeType::Alpha);
	}

	isInitialized_ = true;
}

Vector2 WaterSurfaceComponent::GetWaterMin() const {
	if (!gameObject_) return Vector2(0.0f, 0.0f);	// フォールバック

	Vector3 pos = gameObject_->GetTransform().translate;
	Vector3 scale = gameObject_->GetTransform().scale;

	auto* model = GetModel();
	if (model) {
		const auto& bounds = model->GetModelData()->GetTotalAABB();
		return Vector2(pos.x + bounds.min.x * scale.x, pos.z + bounds.min.z * scale.z);
	}

	return Vector2(pos.x - 50.0f * scale.x, pos.z - 50.0f * scale.z);
}

Vector2 WaterSurfaceComponent::GetWaterSize() const {
	if (!gameObject_) return Vector2(1.0f, 1.0f);

	Vector3 scale = gameObject_->GetTransform().scale;
	auto* model = GetModel();
	if (model) {
		auto& meshes = model->GetModelData()->meshes;
		if (!meshes.empty()) {
			const auto& bounds = meshes[0].localBounds;

			// ★ (max - min) で元のサイズを出して、スケールを掛ける！
			float realWidth = (bounds.max.x - bounds.min.x) * scale.x;
			float realDepth = (bounds.max.z - bounds.min.z) * scale.z;

			return Vector2(realWidth, realDepth);
		}
	}

	return Vector2(100.0f * scale.x, 100.0f * scale.z); // フォールバック
}
