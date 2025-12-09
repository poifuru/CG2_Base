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
		// [ 考える ]
		Vector2 ariaSize = enemy_->GetAriaSize();
		Vector3 ariaLeftTop = enemy_->GetAriaLeftTop();
		Vector3 ariaRightBottom = ariaLeftTop + Vector3(ariaSize.x, 0.0f, ariaSize.y);
		Vector3 enemyPos = enemy_->GetPosition();
		float enemyToUpDistance = (ariaLeftTop.z - enemyPos.z);
		float enemyToDownDistance = (enemyPos.z - ariaRightBottom.z);
		float enemyToLeftDistance = (enemyPos.x - ariaLeftTop.x);
		float enemyToRightDistance = (ariaRightBottom.x - enemyPos.x);
		float deltaTime = 1.0f / 60.0f * 3.0f;

		Vector3 moveAmount{};
		if (enemyToLeftDistance > enemyToRightDistance) {
			// ○○●○
			moveAmount.x = -0.85f * deltaTime;
		}
		else if (enemyToLeftDistance < enemyToRightDistance) {
			// ○●○○
			moveAmount.x = 0.85f * deltaTime;
		}
		else if (enemyToLeftDistance == enemyToRightDistance) {
			moveAmount.x = 0.1f * deltaTime;
		}
		if (enemyToUpDistance > enemyToDownDistance) {
			// ○○●○
			moveAmount.z = 0.5f * deltaTime;
		}
		else if (enemyToUpDistance < enemyToDownDistance) {
			// ○●○○
			moveAmount.z = -0.5f * deltaTime;
		}
		else if (enemyToUpDistance == enemyToDownDistance) {
			moveAmount.z = 0.1f * deltaTime;
		}
		enemy_->SetMoveAmount(moveAmount);
		// 索敵に遷移
		enemy_->ChangeState(new EnemySearchPlayerState());
		return;
	}
#ifdef _DEBUG
	/*ImGui::Begin("Enemy : Decision");
	ImGui::End();*/
#endif//_DEBUG
}

void EnemyDecisionState::Exit() {
	// 元に戻したいとかいう処理は特に無し

}