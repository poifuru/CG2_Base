#pragma once
#include "Component.h"

class ReticleComponent : public Component {
public:
	ReticleComponent() = default;
	~ReticleComponent() override = default;

	void Initialize() override;
	void Update() override;
	void ImGui() override;

	void Serialize(json& j) const override;
	void Deserialize(const json& j) override;

	const char* GetName() const override { return "ReticleComponent"; }

	// ロックオンしている敵オブジェクトを返すゲッター
	GameObject* GetLockOnTarget() const { return lockOnTarget_; }

	// ロックオン有効最大距離（ハープーン射程）の設定
	void SetLockOnMaxDistance(float dist) { lockOnMaxDistance_ = dist; }
	float GetLockOnMaxDistance() const { return lockOnMaxDistance_; }

private:
	float offsetZ_ = 100.0f; // カメラの正面方向への配置距離（m）
	float lockOnAngleCos_ = 0.995f; // ロックオンのしきい値
	float lockOnMaxDistance_ = 18.0f; // ロックオン有効最大距離（ハープーン射程）
	bool isInitialized_ = false; // 初期化済みフラグ

	GameObject* lockOnTarget_ = nullptr; // 現在ロックオンしている敵
};