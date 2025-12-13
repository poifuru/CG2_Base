#include "EnemyState.h"
#include "../Enemy.h"
#include "MathFunction.h"
#include "imgui.h"

void EnemyPreExplosionState::Initialize() {
	// 初期化処理
	// [ タイマーの初期化 ]
	preExplosionTimer_ = 0.0f;// 爆発までのタイマー

	// 他に何かあればここに
	// アタックレベルで半径を設定
	enemy_->SetAttackRadiusForLevel();
}

void EnemyPreExplosionState::Update() {
	// 爆発までの最終接近と警告の処理
	
	// 1, タイマーを進行させる
	float deltaTime = 1.0f / 60.0f;
	preExplosionTimer_ += deltaTime;

	// 2, プレイヤーに向かって接近
	Vector3 enemyPos = enemy_->GetPosition();
	Vector3 playerPos = enemy_->GetTarget()->GetPosition();
	Vector3 toPlayer = playerPos - enemyPos;
	toPlayer.y = 0.0f;

	// [ 距離によって移動させる ]
	if (Math::Length(toPlayer) > 0.01f) {
		Vector3 direction = Math::Normalize(toPlayer);
		Vector3 moveAmount = direction * (chaseSpeedRate_ * deltaTime);
		// ほぼ同じ位置なら移動しない
		enemy_->SetMoveAmount(moveAmount);
	}
	else {
		// ほぼ同じ位置なら移動しない
		enemy_->SetMoveAmount({0.0f,0.0f,0.0f});
	}

	// 3, タイマー終了のチェック
	if (preExplosionTimer_ >= maxPreExplosionTime_) {
		// タイマーが切れたら爆発Stateに移行
		enemy_->ChangeState(new EnemyExplosionState());
		return;
	}
#ifdef _DEBUG
	/*ImGui::Begin("Enemy : PreExplosion");
	ImGui::End();*/
#endif//_DEBUG
}

void EnemyPreExplosionState::Exit() {
	// 終了処理
	
}