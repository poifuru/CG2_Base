#include "EnemyState.h"
#include "../Enemy.h"
#include "MathFunction.h"
#include "imgui.h"

void EnemyHurtState::Initialize() {
	// 初期化処理

	// [ パラメータの初期化 ]
	boundCounter_ = 0.0f;
	attackLevel_ = enemy_->GetAttackLevel();
	// バウンドの回数によって変わる処理
	if (attackLevel_ == 1.0f) {
		boundCounter_ = 1.0f;
	}
	if (attackLevel_ == 2.0f) {
		boundCounter_ = 2.0f;
	}
	if (attackLevel_ >= 3.0f) {
		boundCounter_ = 3.0f;
	}

	moveAmount_ = enemy_->GetKnockBackDirection();
}

void EnemyHurtState::Update() {
	// ダメージを受けた際の処理
	float deltaTime = 1.0f / 60.0f;

	// 1, 方向の更新処理
	float moveSpeed = enemy_->GetMoveSpeed();

	// [ 減速 ]
	moveSpeed -= moveSpeed * (decelerationRate_ * deltaTime);

	// 2, 壁との当たり判定処理
	// 壁に当たると速度は少し低下して反転
	// さらにAtaackの威力も増加する
	if (enemy_->GetPosition().z >= enemy_->GetAriaLeftTop().z || enemy_->GetPosition().z <= enemy_->GetAriaLeftTop().z + enemy_->GetAriaSize().y) {
		moveAmount_.z *= -1.0f;
		moveSpeed *= e_;
		boundCounter_++;
	}
	if (enemy_->GetPosition().x >= enemy_->GetAriaLeftTop().x + enemy_->GetAriaSize().x || enemy_->GetPosition().x <= enemy_->GetAriaLeftTop().x) {
		moveAmount_.x *= -1.0f;
		moveSpeed *= e_;
		boundCounter_++;
	}

	// [ 適用 ]
	enemy_->SetMoveSpeed(moveSpeed);
	enemy_->SetMoveAmount(moveAmount_);

	// バウンドの回数によって変わる処理
	if (boundCounter_ == 1.0f) {
		attackLevel_ = 1.0f;
	}
	if (boundCounter_ == 2.0f) {
		attackLevel_ = 2.0f;
	}
	if (boundCounter_ >= 3.0f) {
		attackLevel_ = 3.0f;
	}
	// 攻撃の威力を設定してあげる
	enemy_->SetAttackLevel(attackLevel_);

	// 3, 速度が一定以下になったらPreExplosionStateに移行
	if (moveSpeed <= minSpeed_) {
		enemy_->ChangeState(new EnemyPreExplosionState());
		return;
	}
#ifdef _DEBUG
	ImGui::Begin("Enemy : Hurt");
	ImGui::End();
#endif//_DEBUG
}

void EnemyHurtState::Exit() {
	// 終了処理
	// 攻撃の威力を設定してあげる
	enemy_->SetAttackLevel(attackLevel_);
}