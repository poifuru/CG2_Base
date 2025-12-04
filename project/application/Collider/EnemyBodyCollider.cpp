#include "EnemyBodyCollider.h"
#include "../application/Enemy/Enemy.h"
#include "MathFunction.h"

EnemyBodyCollider::EnemyBodyCollider(Enemy* enemy) : enemy_(enemy)
{
	// 自身の属性: プレイヤー本体
	SetMyType(COL_Enemy);

	// 衝突対象: 敵の攻撃
	SetYourType(COL_Player_Attack);

	// デフォルトの当たり判定半径を設定
	SetRadius(1.0f);
}

void EnemyBodyCollider::OnCollision(Collider* other)
{
	// 衝突相手を確認して処理を行う
	
	Vector3 enemyPos = GetWorldPosition();
	float damage = 2.0f;

	// 衝突相手が COL_Enemy_Attack 属性を持っているか確認
	if (other->GetMyType() & COL_Player_Attack){
		Vector3 playerPos = other->GetWorldPosition();
		Vector3 playerToEnemy = (enemyPos - playerPos);
		enemy_->SetKnockBackDirection(Normalize(playerToEnemy));

		// プレイヤーに新しく三つの属性を付与させるー＞その属性で攻撃をどのくらい溜めたのか判断する
	}
	if (other->GetMyType() & COL_Enemy) {

	}
	if (other->GetMyType() & COL_Enemy_SlipDamage) {

	}
	if (other->GetMyType() & COL_Boss) {

	}
	if (other->GetMyType() & COL_Boss_Attack) {

	}

	enemy_->TakeDamage(damage);
}

const Vector3 EnemyBodyCollider::GetWorldPosition()
{
	if (!enemy_) return { 0.0f, 0.0f, 0.0f };
	// エネミーのポジションを返す
	return enemy_->GetPosition();
}