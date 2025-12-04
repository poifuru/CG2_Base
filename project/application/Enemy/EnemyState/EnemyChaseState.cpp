#include "EnemyState.h"
#include "../Enemy.h"
#include "MathFunction.h"
#include "imgui.h"

void EnemyChaseState::Initialize() {
	// 初期化処理

}

void EnemyChaseState::Update() {
	// [ プレイヤーに近づく処理 ]

	float deltaTime = 1.0f / 60.0f;

	// 1, プレイヤーとの位置関係を取得
	Vector3 enemyPos = enemy_->GetPosition();
	Vector3 playerPos = enemy_->GetTarget()->GetPosition();

	Vector3 toPlayer = playerPos - enemyPos;
	float distanceToPlayer = Length(toPlayer);

	// 2, プレイヤーに向かって移動する
	Vector3 direction = Normalize(toPlayer);
	Vector3 moveVector = direction * (chaseSpeedRate_ * deltaTime);

	// 3, 移動量を設定
	enemy_->SetMoveAmount(moveVector);

	// 4, 爆発開始距離のチェック
	if (distanceToPlayer <= startExplosionDist_) {
		// 設定した距離いないに入ったら爆発Stateに遷移
		enemy_->ChangeState(new EnemyPreExplosionState());
		return;
	}
#ifdef _DEBUG
	ImGui::Begin("Enemy : Chase");
	ImGui::End();
#endif//_DEBUG
}

void EnemyChaseState::Exit() {
	// 終了処理

}