#include "BossBodyCollider.h"
// Bossクラスのヘッダーをインクルード
#include "Boss.h" 

BossBodyCollider::BossBodyCollider(Boss* boss) : boss_(boss) {
	// 自身の属性: Boss本体
	SetMyType(COL_Boss);

	// 衝突対象: プレイヤーの攻撃全て
	// COL_Player_Attack は全てのプレイヤー攻撃の親属性としています
	SetYourType(COL_Player_Attack | COL_Enemy_Attack | COL_Enemy | COL_Player);

	// Bossの当たり判定半径を設定 (※モデルのサイズに合わせて調整してください)
	SetRadius(7.0f);
}

// 自身のワールド座標を返す
const Vector3 BossBodyCollider::GetWorldPosition() {
	if (!boss_) return { 0.0f, 0.0f, 0.0f };
	// BossクラスのGetPosition()を使用
	return boss_->GetPosition();
}

void BossBodyCollider::OnCollision(Collider* other) {
	if (!boss_) return;

	float damage = 0.0f;

	// 1. **衝突相手がプレイヤーの攻撃であるかチェック**
	if (other->GetMyType() & COL_Player_Attack) {
		// 2. **ダメージ値を決定**
		// PlayerBodyCollider.cppの例を参考に、攻撃レベルに応じてダメージを設定します (暫定値)

		if (other->GetMyType() & COL_Player_Attack_Level0) {
			damage = 1.0f;
		} else if (other->GetMyType() & COL_Player_Attack_Level1) {
			damage = 0.75f;
		} else if (other->GetMyType() & COL_Player_Attack_Level2) {
			damage = 2.0f;
		} else if (other->GetMyType() & COL_Player_Attack_Level3) {
			damage = 4.0f;
		} else {
			// その他の攻撃、またはレベル未設定の攻撃
			damage = 1.0f;
		}
	}
	if (other->GetMyType() & COL_Enemy_Attack){
		if (boss_->GetHP() > boss_->GetMaxHP() * 0.5) {
			// [ Enemyの爆発の場合 ]
			if (other->GetMyType() & COL_Enemy_Attack_Level0) {
				damage = 20.0f;
			}
			if (other->GetMyType() & COL_Enemy_Attack_Level1) {
				damage = 24.0f;
			}
			if (other->GetMyType() & COL_Enemy_Attack_Level2) {
				damage = 28.0f;
			}
			if (other->GetMyType() & COL_Enemy_Attack_Level3) {
				damage = 32.0f;
			}

			// [ Enemyが死んだ際に出すスリップダメージの場合 ]
			if (other->GetMyType() & COL_Enemy_SlipDamage) {
				damage = 0.3f;
			}
		} else if (boss_->GetHP() > boss_->GetMaxHP() * 0.3) {
			// [ Enemyの爆発の場合 ]
			if (other->GetMyType() & COL_Enemy_Attack_Level0) {
				damage = 9.0f;
			}
			if (other->GetMyType() & COL_Enemy_Attack_Level1) {
				damage = 12.0f;
			}
			if (other->GetMyType() & COL_Enemy_Attack_Level2) {
				damage = 15.0f;
			}
			if (other->GetMyType() & COL_Enemy_Attack_Level3) {
				damage = 18.0f;
			}

			// [ Enemyが死んだ際に出すスリップダメージの場合 ]
			if (other->GetMyType() & COL_Enemy_SlipDamage) {
				damage = 0.2f;
			}
		} else {
			// [ Enemyの爆発の場合 ]
			if (other->GetMyType() & COL_Enemy_Attack_Level0) {
				damage = 1.0f;
			}
			if (other->GetMyType() & COL_Enemy_Attack_Level1) {
				damage = 3.0f;
			}
			if (other->GetMyType() & COL_Enemy_Attack_Level2) {
				damage = 5.0f;
			}
			if (other->GetMyType() & COL_Enemy_Attack_Level3) {
				damage = 8.0f;
			}

			// [ Enemyが死んだ際に出すスリップダメージの場合 ]
			if (other->GetMyType() & COL_Enemy_SlipDamage) {
				damage = 0.1f;
			}
		}
	} 

	// 3. **Bossにダメージを適用**
	if (damage > 0.0f) {
		boss_->TakeDamage(damage);
	}
}