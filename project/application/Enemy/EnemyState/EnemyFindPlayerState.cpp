#include "EnemyState.h"
#include "../Enemy.h"
#include "imgui.h"
#include "MathFunction.h"

void EnemyFindPlayerState::Initialize() {
	// 初期化処理
	// [ パラメータの初期化 ]
	findTimer_ = 0.0f;

	// 何か見つけたときにアクションを起こす場合はここに書く
	// [ 回転させたいので目線が追従しないようにする ]
	enemy_->SetIsViewPlayerFlag(false);

	// [ 移動量を設定？ ]
}

void EnemyFindPlayerState::Update() {
		// プレイヤーを見つけた時の処理
	findTimer_ += 1.0f / 60.0f;

	Vector3 rot = enemy_->GetRotation();
	rot.y += Math::Deg2Rad(360.0f / (60.0f / maxFindTime_));
	enemy_->SetRotation(rot);

	// 一定時間経過したら、Chase State に遷移
	if (findTimer_ >= maxFindTime_) {
		enemy_->ChangeState(new EnemyChaseState());
		return;
	}
#ifdef _DEBUG
	ImGui::Begin("Enemy : Find");
	ImGui::End();
#endif//_DEBUG
}

void EnemyFindPlayerState::Exit() {
	// 終了処理
	// [ 追従処理を再開する ]
	enemy_->SetIsViewPlayerFlag(true);
}
