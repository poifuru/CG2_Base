#include "EnemyState.h"
#include "../Enemy.h"
#include "imgui.h"

void EnemyFindPlayerState::Initialize() {
	// 初期化処理
	// [ パラメータの初期化 ]
	findTimer_ = 0.0f;

	// 何か見つけたときにアクションを起こす場合はここに書く

	// [ 移動量を設定？ ]
}

void EnemyFindPlayerState::Update() {
		// プレイヤーを見つけた時の処理
	findTimer_ += 1.0f / 60.0f;
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

}
