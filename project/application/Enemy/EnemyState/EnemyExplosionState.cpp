#include "EnemyState.h"
#include "../Enemy.h"
#include "imgui.h"

void EnemyExplosionState::Initialize() {
	// 初期化処理
	// [ タイマーの初期化 ]
	explosionTimer_ = 0.0f;

	// [ 移動量 ]
	enemy_->SetMoveAmount({ 0.0f,0.0f,0.0f });

	// アタックレベルで半径を設定
	enemy_->SetAttackRadiusForLevel();

	// 当たり判定を設定
	enemy_->EnableHitBox(true,enemy_->GetPosition());

	// Enemyは死んだ
	enemy_->SetIsAlive(false);

	audio_.Initialize();
	explosionHandle_ = audio_.LoadSound("resources/Audio/SE/explosion.mp3");
	audio_.PlaySoundW(explosionHandle_, 1.7f, false);
}

void EnemyExplosionState::Update() {
	// 爆発の処理
	// [ タイマーの進行 ]
	explosionTimer_ += 1.0f / 60.0f;

	// [ 爆発の終了 ]
	if (explosionTimer_ >= maxExplosionTime_) {
		
		enemy_->ChangeState(new EnemyDeathState());
		return;
	}
#ifdef _DEBUG
	/*ImGui::Begin("Enemy : Explosion");
	ImGui::End();*/
#endif//_DEBUG
}

void EnemyExplosionState::Exit() {
	// 終了処理
	enemy_->EnableHitBox(false, {0.0f,0.0f,0.0f});
	audio_.Unload(explosionHandle_);
}