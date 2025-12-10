#include "PlayerBodyCollider.h"
#include "../application/Player/Player.h"

PlayerBodyCollider::PlayerBodyCollider(Player* player) : player_(player)
{
	// 自身の属性: プレイヤー本体
	SetMyType(COL_Player);

	// 衝突対象: 敵の攻撃
	SetYourType(COL_Enemy_Attack | COL_Boss_Attack | COL_Boss);

	// デフォルトの当たり判定半径を設定
	SetRadius(1.0f);
}

void PlayerBodyCollider::OnCollision(Collider* other)
{
	if (!player_) return;
	float damage = 0.0f;
	// 1. **衝突相手が敵の攻撃であるかチェック**
	// 衝突相手が COL_Enemy_Attack 属性を持っているか確認
	if (other->GetMyType() & COL_Enemy_Attack)
	{
		// 既に死亡状態や無敵時間中の場合はダメージ処理を行わない
		// if (player_->IsDead() || player_->IsInvulnerable()) { return; }

		// 2. **ダメージ値を決定**
		// Enemyなら攻撃力を取得、もしくはRadiusから計算other->GetRadius()

		// [ Enemyの爆発の場合 ]
		if (other->GetMyType() & COL_Enemy_Attack_Level0) {
			damage = 3.0f;
		}
		if (other->GetMyType() & COL_Enemy_Attack_Level1) {
			damage = 6.0f;
		}
		if (other->GetMyType() & COL_Enemy_Attack_Level2) {
			damage = 9.0f;
		}
		if (other->GetMyType() & COL_Enemy_Attack_Level3) {
			damage = 15.0f;
		}

		// [ Enemyが死んだ際に出すスリップダメージの場合 ]
		if (other->GetMyType() & COL_Enemy_SlipDamage) {
			damage = 5.0f;
		}
	}
	if (other->GetMyType() & COL_Boss_Attack) {
		if (other->GetMyType() & COL_Boss_Attack_CenterStomp) {
			damage = 50.0f;
		}
		if (other->GetMyType() & COL_Boss_Attack_FullScreenBullet) {
			damage = 20.0f;
		}
		if (other->GetMyType() & COL_Boss_Attack_Breath) {
			damage = 10.0f;
		}
	}
	if (other->GetMyType() & COL_Boss) {
		damage = 0.5f;
	}
	// 3. **Player3D 本体にダメージを通知**
	// Player3D に TakeDamage(float) メソッドが必要です
	player_->TakeDamage(damage);
}

const Vector3 PlayerBodyCollider::GetWorldPosition()
{
	if (!player_) return { 0.0f, 0.0f, 0.0f };
	// Player3D::GetPosition() はプレイヤーのワールド座標を返す想定
	return player_->GetPosition();
}