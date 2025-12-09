#include "EnemyBodyCollider.h"
#include "../application/Enemy/Enemy.h"
#include "MathFunction.h"

EnemyBodyCollider::EnemyBodyCollider(Enemy* enemy) : enemy_(enemy)
{
	// 自身の属性: プレイヤー本体
	SetMyType(COL_Enemy);

	// 衝突対象: 敵の攻撃
	SetYourType(COL_Player_Attack | COL_Enemy | COL_Enemy_SlipDamage | COL_Enemy_Attack | COL_Boss_Attack | COL_Boss);

	// デフォルトの当たり判定半径を設定
	SetRadius(1.0f);
}

void EnemyBodyCollider::OnCollision(Collider* other)
{
	if (enemy_->GetIsAlive() == false) {
		return;
	}

	// 衝突相手を確認して処理を行う
	
	Vector3 enemyPos = GetWorldPosition();
	float damage = 0.5f;

	// 衝突相手が COL_Enemy_Attack 属性を持っているか確認
	if (other->GetMyType() & COL_Player_Attack){
		// プレイヤーの攻撃が当たったかどうかをプラスする
		enemy_->AddBodyHitType(COL_Enemy_Attack_From_Player);

		Vector3 playerPos = other->GetWorldPosition();
		Vector3 playerToEnemy = (enemyPos - playerPos);
		playerToEnemy.y = 0.0f;
		enemy_->SetKnockBackDirection(Math::Normalize(playerToEnemy));

		// プレイヤーに新しく三つの属性を付与させるー＞その属性で攻撃をどのくらい溜めたのか判断する
		// 通常攻撃
		if (other->GetMyType() & COL_Player_Attack_Level0) {
			damage = 0.5f;
		}
		// 溜め攻撃
		// [ Level1 ]
		if (other->GetMyType() & COL_Player_Attack_Level1) {
			damage = 0.35f;
		}
		// [ Level2 ]
		if (other->GetMyType() & COL_Player_Attack_Level2) {
			damage = 1.0f;
		}
		// [ Level3 ]
		if (other->GetMyType() & COL_Player_Attack_Level3) {
			damage = 2.0f;
		}

		enemy_->TakeDamage(damage);
		return;
	}

	else if (other->GetMyType() & COL_Enemy) {
		Vector3 enemyEnemyPos = other->GetWorldPosition();
		Vector3 enemyToEnemy = (enemyPos - enemyEnemyPos);
		enemyToEnemy.y = 0.0f;
		enemy_->SetKnockBackDirection(Math::Normalize(enemyToEnemy));
		damage = -0.1f;

		enemy_->TakeDamage(damage);
		return;
	}
	else if (other->GetMyType() & COL_Enemy_Attack) {
		if (other->GetMyType() & COL_Enemy_Attack_Level0) {
			damage = 2.0f;
		}
		if (other->GetMyType() & COL_Enemy_Attack_Level1) {
			damage = 3.0f;
		}
		if (other->GetMyType() & COL_Enemy_Attack_Level2) {
			damage = 4.5f;
		}
		if (other->GetMyType() & COL_Enemy_Attack_Level3) {
			damage = 6.0f;
		}
		Vector3 enemyEnemyPos = other->GetWorldPosition();
		Vector3 enemyToEnemy = (enemyPos - enemyEnemyPos);
		enemyToEnemy.y = 0.0f;
		enemy_->SetKnockBackDirection(Math::Normalize(enemyToEnemy));

		enemy_->TakeDamage(damage / 20.0f);
		return;
	}
	else if (other->GetMyType() & COL_Enemy_SlipDamage) {
		// 喰らったら即PreExplosion！！！！！！
		enemy_->TakeSlipDamage();
	}
	else if (other->GetMyType() & COL_Boss_Attack) {
		if (other->GetMyType() & COL_Boss_Attack_CenterStomp) {

			Vector3 bossPos = other->GetWorldPosition();
			Vector3 bossToEnemy = (enemyPos - bossPos);
			bossToEnemy.y = 0.0f;
			enemy_->SetKnockBackDirection(Math::Normalize(bossToEnemy));

			// 喰らったら吹っ飛ぶ
			damage = 2.75f;
			enemy_->TakeDamage(damage);
			return;
		}
	}
	else if (other->GetMyType() & COL_Boss) {
		if (GetMyType() & COL_Enemy_Attack_From_Player) {
			enemy_->TakeBossDamage();
			return;
		}
	}
}

const Vector3 EnemyBodyCollider::GetWorldPosition()
{
	if (!enemy_) return { 0.0f, 0.0f, 0.0f };
	// エネミーのポジションを返す
	return enemy_->GetPosition();
}