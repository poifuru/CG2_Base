#pragma once
#include "Collider.h"
#include "struct.h" // Vector3, Transform のため

// Bossが発射する弾丸の当たり判定
class BossProjectileCollider final : public Collider {
public:
	// コンストラクタ: 自身の座標を外部から受け取るためのポインタとダメージ量
	BossProjectileCollider(const Vector3* positionPtr, float damage, COLLISIONATTRIBUTE type);
	void OnCollision(Collider* other) override;
	const Vector3 GetWorldPosition() override;
private:
	const Vector3* positionPtr_ = nullptr; // 座標を外部から参照するためのポインタ
	float damage_ = 0.0f;
};