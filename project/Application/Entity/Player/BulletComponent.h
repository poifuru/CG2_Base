#pragma once
#include "Component.h"

class BulletComponent : public Component {
public:
	BulletComponent() = default;
	~BulletComponent() override = default;

	void Initialize() override;
	void Update() override;

	const char* GetName() const override { return "BulletComponent"; }
	void SetDirection(const Vector3& dir) { direction_ = dir; }
	void SetSpeed(float speed) { speed_ = speed; }
	void SetHomingStrength(float strength) { homingStrength_ = strength; }

	void SetTarget(GameObject* target) { target_ = target; }

private:
	Vector3 direction_{};			// 向き
	float speed_ = 80.0f;			// スピード
	float activeTimer_ = 3.0f;		// 有効時間
	float homingStrength_ = 0.1f;	// 追尾力(%)

	GameObject* target_ = nullptr; // 追尾対象
};