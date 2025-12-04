#include "EnemyState.h"
#include "../Enemy.h"
#include "MathFunction.h"
#include "imgui.h"

void EnemyDecisionState::Initialize() {
	// 初期化処理
	// [ 移動量を０にする ]
	enemy_->SetMoveAmount({0.0f,0.0f,0.0f});
	// [ パラメータの初期化 ]
	decisionTimer_ = 0.0f;
}

void EnemyDecisionState::Update() {
	// ここにどんな行動をするかのAIを設定する

	// [ 時間経過の処理 ]
	decisionTimer_ += 1.0f / 60.0f;

	// [ 考える処理 ]
	if (decisionTimer_ <= maxDecisionTime_) {
		// プレイヤーが目の前に居たらFindに遷移

	}
	// [ 時間を過ぎたら強制的に索敵に移行 ]
	else {
		// 索敵に遷移
		enemy_->ChangeState(new EnemySearchPlayerState());
		return;
	}
#ifdef _DEBUG
	ImGui::Begin("Enemy : Decision");
	ImGui::End();
#endif//_DEBUG
}

void EnemyDecisionState::Exit() {
	// 元に戻したいとかいう処理は特に無し

}