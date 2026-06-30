#include "PCH.h"
#include "BirdEnemyComponent.h"
#include "GameObject.h"
#include "DeltaTime.h"

void BirdEnemyComponent::Initialize() {
	if (isInitialized_) return;
	isInitialized_ = true;

	if (gameObject_) {
		center_ = gameObject_->GetTransform().translate; // 配置された初期位置を中心にする
	}

	radius_ = 10.0f;
	speed_ = 1.0f;
	angle_ = 0.0f;
}
void BirdEnemyComponent::Update() {
	if (!gameObject_) return;

	// 角度を回転させる
	angle_ += speed_ * kDeltaTime;
	if (angle_ > 3.14159265f * 2.0f) {
		angle_ -= 3.14159265f * 2.0f;
	}

	// 円の軌道上の座標を計算
	Vector3 nextPos = {
		center_.x + radius_ * std::cos(angle_),
		center_.y, // 高さは固定
		center_.z + radius_ * std::sin(angle_)
	};
	gameObject_->GetTransform().translate = nextPos;

	// 円運動の接線方向（進行方向）に向くように回転（Yaw）を設定
	gameObject_->GetTransform().rotate.y = -angle_ + 1.570796f; // ※モデルの初期の向きに合わせて微調整してね！
}
void BirdEnemyComponent::ImGui() {
	ImGui::DragFloat3("Center", &center_.x, 0.1f);
	ImGui::DragFloat("Radius", &radius_, 0.1f, 0.0f, 100.0f);
	ImGui::DragFloat("Speed", &speed_, 0.05f, -10.0f, 10.0f);
}
void BirdEnemyComponent::Serialize(json& j) const {
	j["type"] = "BirdEnemyComponent";
	j["center"] = { center_.x, center_.y, center_.z };
	j["radius"] = radius_;
	j["speed"] = speed_;
	j["angle"] = angle_;
}
void BirdEnemyComponent::Deserialize(const json& j) {
	isInitialized_ = true;
	if (j.contains("center")) {
		center_ = { j["center"][0], j["center"][1], j["center"][2] };
	}
	if (j.contains("radius")) radius_ = j["radius"];
	if (j.contains("speed")) speed_ = j["speed"];
	if (j.contains("angle")) angle_ = j["angle"];
}