#include "CenterStompCollider.h"
#include "CenterStomp.h" 
#include "Player.h" // Playerの座標取得のため
#include "PlayerBodyCollider.h" // OnCollisionのロジックをPlayer側に集約するため

CenterStompCollider::CenterStompCollider(CenterStomp* parentAttack, float damage)
	: parentAttack_(parentAttack), damage_(damage) {
	// 自身の属性: CenterStomp攻撃 (Boss_Attackフラグも立てる)
	SetMyType(COL_None);

	// 衝突対象: プレイヤー本体
	SetYourType(COL_None);

	// 当たり判定の半径を設定
	SetRadius(15.0f); // 攻撃範囲に合わせて大きめに設定
}

const Vector3 CenterStompCollider::GetWorldPosition() {
	if (!parentAttack_) return { 0.0f, 0.0f, 0.0f };
	// CenterStompクラスに実装されている GetPosition() を使用
	return parentAttack_->GetPosition();
}

void CenterStompCollider::OnCollision(Collider* other) {
	// CenterStomp攻撃は持続的な範囲攻撃ではないため、
	// 衝突した事実をPlayerBodyCollider側で処理し、この攻撃側では特に何もしなくてOK。
	(void)other;
}