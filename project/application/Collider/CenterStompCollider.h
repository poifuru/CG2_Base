#pragma once
#include "Collider.h"
#include "struct.h" // Vector3のため

class CenterStomp;

// BossのCenterStomp攻撃の当たり判定
class CenterStompCollider final : public Collider {
public:
	CenterStompCollider(CenterStomp* parentAttack, float damage);
	void OnCollision(Collider* other) override;
	const Vector3 GetWorldPosition() override;
private:
	CenterStomp* parentAttack_ = nullptr;
	float damage_ = 0.0f;
};