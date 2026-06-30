#pragma once
#include "Component.h"

class BirdEnemyComponent : public Component {
public:
	BirdEnemyComponent() = default;
	~BirdEnemyComponent() override = default;

	void Initialize() override;
	void Update() override;
	void ImGui() override;

	void Serialize(json& j) const override;
	void Deserialize(const json& j) override;

	const char* GetName() const override { return "BirdEnemyComponent"; }
private:
	Vector3 center_{};     // 円運動の中心
	float radius_ = 10.0f;  // 半径
	float speed_ = 1.0f;    // 回転スピード（ラジアン/秒）
	float angle_ = 0.0f;    // 現在の角度（ラジアン）
	bool isInitialized_ = false; // リセット防止
};