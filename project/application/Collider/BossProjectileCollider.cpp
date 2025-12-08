#include "BossProjectileCollider.h"
#include "PlayerBodyCollider.h"
#include "Player.h"

BossProjectileCollider::BossProjectileCollider(const Vector3* positionPtr, float damage, COLLISIONATTRIBUTE type)
	: positionPtr_(positionPtr), damage_(damage) {
	// 属性は外部から受け取る (例: COL_Boss_Attack_FullScreenBullet)
	SetMyType(type | COL_Boss_Attack);
	SetYourType(COL_Player);
	SetRadius(0.5f); // 弾のサイズに合わせて小さめに設定
}

const Vector3 BossProjectileCollider::GetWorldPosition() {
	if (!positionPtr_) return { 0.0f, 0.0f, 0.0f };
	return *positionPtr_; // 外部から参照している座標を返す
}

void BossProjectileCollider::OnCollision(Collider* other) {
	// 弾は衝突したら、PlayerBodyCollider 側でダメージを与え、
	// PlayerBodyCollider 側で `SetIsActive(false)` を呼び出して弾を消滅させます。
	// このため、このクラス内では特に処理を記述しません。
	(void)other;
}