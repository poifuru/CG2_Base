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

	void SetTarget(GameObject* target) { target_ = target; }

private:
	Vector3 direction_{};
	float speed_ = 80.0f;
	float activeTimer_ = 3.0f;

	GameObject* target_ = nullptr; // 追尾対象
};