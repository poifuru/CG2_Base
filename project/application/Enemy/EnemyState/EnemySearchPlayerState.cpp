#include "EnemyState.h"
#include "../Enemy.h"
#include "MathFunction.h"
#include "imgui.h"

void EnemySearchPlayerState::Initialize() {
	// 初期化処理
	// [ パラメータの初期化 ]
	searchTimer_ = 0.0f;
}

void EnemySearchPlayerState::Update() {
	
	// 索敵処理
	// [ 索敵の時間の経過 ]
	searchTimer_ += 1.0f / 60.0f;

	// 見つけていないときの移動処理(ここをもっと詳しく作る)
	// 考えるところで作っちまった

	// [ プレイヤーを見つけたかの判定(視界内に入ったら見つけたことにする) ]
	Vector3 toPlayer = enemy_->GetTarget()->GetPosition() - enemy_->GetPosition();
	float distanceToPlayer = Math::Length(toPlayer);
	toPlayer = Math::Normalize(toPlayer);

	// [ 視認距離に入っていれば索敵処理を行う ]
	if (distanceToPlayer <= viewDistance_) {
		// 敵の前方向ベクトル
		Vector3 enemyForward = { 0.0f,0.0f,1.0f };
		// 敵の回転を考慮した前方向ベクトルを計算
		float radian = std::atan2(enemy_->GetMoveAmount().x, enemy_->GetMoveAmount().z);
		Matrix4x4 rotMatrix = Math::MakeRotateYMatrix(radian);
		enemyForward = Math::ChangeTransform(enemyForward, rotMatrix);
		enemyForward = Math::Normalize(enemyForward);
		// 視界角度(ラジアン)
		float viewAngle = Math::Deg2Rad(45.0f); // 45度の視界
		// プレイヤーが視界内にいるかどうかの判定
		float dotProduct = Math::Dot(enemyForward, toPlayer);
		float cosViewAngle = cosf(viewAngle);
		if (dotProduct >= cosViewAngle) {
			// プレイヤーを見つけた
			enemy_->ChangeState(new EnemyFindPlayerState());
			return;
		}
	}
	// 索敵時間が最大値を超えたら、意思決定 State に戻る
	if (searchTimer_ >= maxSearchTime_) {
		// 索敵失敗、意思決定に戻る
		enemy_->ChangeState(new EnemyDecisionState());
		return;
	}

#ifdef _DEBUG
	ImGui::Begin("Enemy : Search");
	ImGui::End();
#endif//_DEBUG
}

void EnemySearchPlayerState::Exit() {
	// 終了処理

}