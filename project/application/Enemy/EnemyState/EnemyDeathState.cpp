#include "EnemyState.h"
#include "../Enemy.h"
#include "imgui.h"

void EnemyDeathState::Initialize() {
	// 初期化処理
	// [ タイマーの初期化 ]
	deathTimer_ = 0.0f;

	// モデル描画関係
	enemy_->EnableHitBox(true, enemy_->GetPosition());
}

void EnemyDeathState::Update() {
	// 死亡演出の処理
	deathTimer_ += 1.0f / 60.0f;

	// ここでカラーをだんだん薄くしていくこともあり
	// Easing({1.0f,1.0f,1.0f,1.0f} -> {1.0f,1.0f,1.0f,0.0f});

	// 一定時間経過したら、Enemyを削除する
	if (deathTimer_ >= maxDeathDuration_) {
		// 本当にさよならする -> EnemyManagerで存在をけしてもらう
		enemy_->SetIsFinished(true);
		return;
	}
#ifdef _DEBUG
	ImGui::Begin("Enemy : Death");
	ImGui::End();
#endif//_DEBUG
}

void EnemyDeathState::Exit() {
	// 終了処理
	enemy_->EnableHitBox(false, enemy_->GetPosition());
}