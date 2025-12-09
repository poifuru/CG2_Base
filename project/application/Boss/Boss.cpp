#include "Boss.h"
#include <imgui.h>
#include "Player.h"
#include "MathFunction.h"
#include "ModelManager.h"
#include <numbers>

Boss::Boss (DxCommon* dxCommon, Player* player) {
	model_ = std::make_unique<Model> (DxCommon::GetInstance ());
	ModelManager::GetInstance ()->LoadModelData ("Resources/boss", "boss");
	ModelManager::GetInstance ()->LoadModelData ("Resources/slipDamage", "slipDamage");

	dxCommon_ = dxCommon;
	player_ = player;
	input_ = InputManager::GetInstance ();
}

Boss::~Boss () {

}

void Boss::Initialize () {
	model_->SetModelData ("boss");
	model_->SetTexture ("boss");
	model_->Initialize ();

	centerStomp_ = std::make_unique<CenterStomp> (this);
	centerStomp_->Initialize ();

	fullScreenAttack_ = std::make_unique<FullScreenAttack> (dxCommon_, this);
	fullScreenAttack_->Initialize ();

	Breath_ = std::make_unique<Breath> (dxCommon_, this);
	Breath_->Initialize ();

	bossBodyCollider_ = std::make_unique<BossBodyCollider> (this);
	bodyColliderObj_ = std::make_unique<Model> (dxCommon_);
	bodyColliderObj_->SetModelData ("slipDamage");
	bodyColliderObj_->SetTexture ("slipDamage");
	bodyColliderObj_->Initialize ();
	bodyColliderObj_->SetColor ({ 0.0f,0.0f,0.0f,1.0f });
}

void Boss::Update (Matrix4x4* m) {
	// HPの更新
	UpdateHp ();
	// 攻撃の更新
	UpdateAttack ();
	// 行動の更新
	UpdateMove ();
	// アニメーション
	UpdateRotation ();
	UpdateAnimation ();

	model_->Update (m);
	centerStomp_->Update (m);
	fullScreenAttack_->Update (m);
	Breath_->Update (m, player_->GetPosition ());

	model_->SetTransform (transform_);

	bodyColliderObj_->SetTransform ({ {bossBodyCollider_->GetRadius () * 0.4f, 0.1f, bossBodyCollider_->GetRadius () * 0.4f},{0.0f,0.0f,0.0f},
		{bossBodyCollider_->GetWorldPosition ().x,
		-0.9f,
		bossBodyCollider_->GetWorldPosition ().z} });

	bodyColliderObj_->Update (m);
}

void Boss::Draw () {
	model_->Draw ();
	bodyColliderObj_->Draw ();
	centerStomp_->Draw ();
	fullScreenAttack_->Draw ();
	Breath_->Draw ();
}

void Boss::ImGuiControl () {
#ifdef _DEBUG
	model_->ImGui ("boss");
	centerStomp_->ImGuiControl ();
	fullScreenAttack_->ImGuiControl ();
	Breath_->ImGuiControl ();

	ImGui::Begin ("Status");
	if (ImGui::BeginTabBar ("StatusTabBar")) {
		if (ImGui::BeginTabItem ("HP")) {
			ImGui::Text ("HP = %.0f/%.0f", hp_, maxHP_);
			if (ImGui::Button ("Reset[HP]")) {
				hp_ = maxHP_;
			}
			if (ImGui::Button ("damage-- [100]")) {
				hp_ -= 100;
			}
			if (ImGui::Button ("heal++   [100]")) {
				hp_ += 100;
			}
			ImGui::EndTabItem ();
		}
		ImGui::EndTabBar ();
	}
	ImGui::End ();
#endif
}

bool Boss::IsAnyAttackActive () const {
	return centerStomp_->IsAttacking () || fullScreenAttack_->IsAttacking ();
}

void Boss::UpdateMove () {
	if (IsAnyAttackActive ()) {
		return;
	}

	// ブレス攻撃しているときの動き
	BreathMove ();

	// 行動パターン
	NormalMove ();
}

void Boss::UpdateAttack () {
	if (input_->GetRawInput ()->Trigger ('1')) {
		centerStomp_->StartAttack ();
	}
	if (input_->GetRawInput ()->Trigger ('2')) {
		fullScreenAttack_->StartAttack ();
	}
	if (input_->GetRawInput ()->Trigger ('3')) {
		Breath_->StartAttack (150, 0.001f);
	}
}

void Boss::BreathMove () {
	if (Breath_->IsAttacking ()) {
		// 1. プレイヤーの座標を取得する
		Vector3 playerPosition = player_->GetPosition ();
		Vector3 bossPosition = transform_.translate;

		// 2. プレイヤーへの方向ベクトルを計算する (プレイヤーの位置 - ボスの位置)
		// MathFunction.h に Subtract 関数が定義されていると仮定
		Vector3 toPlayerVector = Math::Subtract (playerPosition, bossPosition);

		// 3. 方向ベクトルを正規化する
		float len = Math::Length (toPlayerVector);
		if (len < 0.0001f) { return; }
		Vector3 direction = Math::Normalize (toPlayerVector);

		// 4. 方向ベクトルに追尾速度を掛けて、今回のフレームの移動量を計算する
		// MathFunction.h に Multiply 関数が定義されていると仮定
		Vector3 moveAmount = Math::Multiply (speed_, direction);

		// 5. ボスの位置を更新する
		transform_.translate.x += moveAmount.x;
		transform_.translate.y += moveAmount.y;
		transform_.translate.z += moveAmount.z;
	}
}

void Boss::NormalMove () {
	// 攻撃中でない場合、移動ステートを更新・実行
	UpdateMoveState (); // ステートを切り替えるロジック

	if (!Breath_->IsAttacking ()) {
		switch (moveState_) {
		case MoveState::Wander:
			WanderMove ();
			break;
		case MoveState::Follow:
			FollowMove ();
			break;
		case MoveState::Evade:
			EvadeMove ();
			break;
		default:
			break;
		}
	}
}

void Boss::WanderMove () {
	// 一定フレームごと、または目標に近づいたら新しい目標座標を設定
	if (wanderUpdateCount_ % 120 == 0 || Math::Length (Math::Subtract (wanderTargetPos_, transform_.translate)) < 1.0f) {

		// 目標タイプをランダムに決定し、動きを読みにくくする
		wanderGoalType_ = static_cast<WanderGoalType>(rand () % 3);

		if (wanderGoalType_ == WanderGoalType::Center) {
			// マップ中央へ向かう
			wanderTargetPos_ = { 0.0f, 0.0f, 0.0f };
		}
		else if (wanderGoalType_ == WanderGoalType::MapEdge) {
			// マップ端のランダムな位置へ向かう (例: X/Z -25から25の範囲をマップ端と仮定)
			wanderTargetPos_.x = static_cast<float>(rand () % 51 - 25);
			wanderTargetPos_.z = static_cast<float>(rand () % 51 - 25);
		}
		else { // WanderGoalType::LocalRandom
			// 現在地付近のランダムな位置へ向かう (例: 現在地から±5の範囲)
			wanderTargetPos_.x = transform_.translate.x + static_cast<float>(rand () % 11 - 5);
			wanderTargetPos_.z = transform_.translate.z + static_cast<float>(rand () % 11 - 5);
		}

		wanderUpdateCount_ = 0;
	}

	Vector3 toTarget = Math::Subtract (wanderTargetPos_, transform_.translate);
	float len = Math::Length (toTarget);
	if (len < 0.0001f) { return; }
	Vector3 direction = Math::Normalize (toTarget);

	// 目的のタイプによって速度に変化をつける
	float currentSpeed = speed_ * wanderBaseSpeedFactor_;
	if (wanderGoalType_ == WanderGoalType::MapEdge) {
		currentSpeed = speed_ * wanderEdgeSpeedFactor_;
	}

	Vector3 moveAmount = Math::Multiply (currentSpeed, direction);

	transform_.translate.x += moveAmount.x;
	transform_.translate.z += moveAmount.z;

	wanderUpdateCount_++;
}

void Boss::FollowMove () {
	// 1. プレイヤーの座標を取得する
	Vector3 playerPosition = player_->GetPosition ();
	Vector3 bossPosition = transform_.translate;

	// 2. プレイヤーへの方向ベクトルを計算し、正規化する
	Vector3 toPlayerVector = Math::Subtract (playerPosition, bossPosition);
	float len = Math::Length (toPlayerVector);
	if (len < 0.0001f) { return; }
	Vector3 direction = Math::Normalize (toPlayerVector);

	// 3. 方向ベクトルに追尾速度を掛けて、今回のフレームの移動量を計算する
	Vector3 moveAmount = Math::Multiply (speed_, direction);

	// 4. ボスの位置を更新する
	transform_.translate.x += moveAmount.x;
	transform_.translate.z += moveAmount.z;
}

void Boss::EvadeMove () {
	// 1. プレイヤーの座標を取得する
	Vector3 playerPosition = player_->GetPosition ();
	Vector3 bossPosition = transform_.translate;

	// 2. プレイヤーへの方向ベクトルを計算し、正規化する
	Vector3 toPlayerVector = Math::Subtract (playerPosition, bossPosition);
	float len = Math::Length (toPlayerVector);
	if (len < 0.0001f) { return; }
	Vector3 direction = Math::Normalize (toPlayerVector); // プレイヤー方向

	// 3. 離脱なので、プレイヤー方向と逆向きに移動する (-direction)
	Vector3 awayDirection = Math::Multiply (-1.0f, direction);

	// 4. 離脱速度を掛けて、今回のフレームの移動量を計算する
	Vector3 moveAmount = Math::Multiply (speed_, awayDirection);

	// 5. ボスの位置を更新する
	transform_.translate.x += moveAmount.x;
	transform_.translate.z += moveAmount.z;
}

void Boss::UpdateMoveState () {
	Vector3 bossPosition = transform_.translate;
	Vector3 playerPosition = player_->GetPosition ();
	// プレイヤーとの距離
	float distance = Math::Length (Math::Subtract (playerPosition, bossPosition));
	float distanceSq = distance * distance; // 距離の2乗

	moveTimer_++;

	switch (moveState_) {
	case MoveState::Wander:
		// プレイヤーが極端に遠すぎる/近すぎる場合は、緊急でステートを切り替える
		if (distance > followDistance_ * emergencyFollowFactor_) { // 遠すぎたら、一旦近づく
			moveState_ = MoveState::Follow;
			moveTimer_ = 0;
		}
		else if (distance < evadeDistance_ * emergencyEvadeFactor_) { // 近すぎたら、一旦離脱する
			moveState_ = MoveState::Evade;
			moveTimer_ = 0;
		}

		// 自由徘徊時間が終了したら、戦術的移動に切り替える時間を大幅に延長
		else if (moveTimer_ > maxMoveTime_ * wanderTimeFactor_) {
			// 自由徘徊時間が終了したら、次は戦術的移動（Follow/Evade）にランダムで切り替える
			if (rand () % 2 == 0) {
				moveState_ = MoveState::Follow;
			}
			else {
				moveState_ = MoveState::Evade;
			}
			moveTimer_ = 0;
		}

		break;

	case MoveState::Follow:
		// プレイヤーに近づきすぎたら離脱に切り替え
		if (distanceSq < evadeDistance_ * evadeDistance_) {
			moveState_ = MoveState::Evade;
			moveTimer_ = 0;
		}
		// 一定時間追尾したら自由徘徊（Wander）に戻る（戦術的移動フェーズの終了）
		else if (moveTimer_ > maxMoveTime_) {
			moveState_ = MoveState::Wander;
			moveTimer_ = 0;
		}
		break;

	case MoveState::Evade:
		// プレイヤーから適度に離れたら追尾に切り替え（近すぎず遠すぎない距離を保つ）
		if (distanceSq > followDistance_ * followDistance_) {
			moveState_ = MoveState::Follow;
			moveTimer_ = 0;
		}
		// 一定時間離脱したら自由徘徊（Wander）に戻る（戦術的移動フェーズの終了）
		else if (moveTimer_ > maxMoveTime_) {
			moveState_ = MoveState::Wander;
			moveTimer_ = 0;
		}
		break;

	case MoveState::Attack:
		// 攻撃から戻った直後のステートはWanderにしておく
		moveState_ = MoveState::Wander;
		moveTimer_ = 0;
		break;
	}
}

void Boss::UpdateHp () {
	DefineTheHpRange ();
}

// HPの max/min を超えないようにする
void Boss::DefineTheHpRange () {
	if (hp_ < 0.0f) {
		hp_ = 0.0f;
	}
	if (hp_ > maxHP_) {
		hp_ = maxHP_;
	}
}

void Boss::TakeDamage (float damage) {
	hp_ -= damage;
	DefineTheHpRange ();
}

void Boss::UpdateRotation () {
	// 1. プレイヤーとボスの位置を取得
	Vector3 playerPosition = player_->GetPosition ();
	Vector3 bossPosition = transform_.translate;

	// 2. プレイヤーへの方向ベクトルを計算 (XZ平面のみを考慮)
	Vector3 toPlayerVector = Math::Subtract (playerPosition, bossPosition);

	float targetRadian = transform_.rotate.y; // デフォルトは現在の回転を維持

	// ⭐️ 修正案: ブレス攻撃中かどうかのチェックを追加 ⭐️
	if (Breath_->IsAttacking ()) {
		// ブレス攻撃中は強制的にプレイヤーの方向を向く (Followと同じロジック)
		float radian = std::atan2 (toPlayerVector.x, toPlayerVector.z);
		// 180度反転させて正面を向かせる (モデルの向きの調整)
		targetRadian = radian + static_cast<float>(std::numbers::pi);
	}
	else {
		// 通常の移動ステートに基づく回転
		switch (moveState_) {
		case MoveState::Follow:
			// 追尾時: プレイヤーの方向を向く
		{
			// atan2(X成分, Z成分)で角度を求める
			float radian = std::atan2 (toPlayerVector.x, toPlayerVector.z);
			// 180度反転させて正面を向かせる (前回の修正を適用)
			targetRadian = radian + static_cast<float>(std::numbers::pi);
		}
		break;
		// ... (Evade, Wander のロジックは省略) ...
		case MoveState::Evade:
			// 離脱時: プレイヤーに背を向ける
		{
			// プレイヤーから離れるベクトル (-toPlayerVector) を使う
			float evadeRadian = std::atan2 (-toPlayerVector.x, -toPlayerVector.z);
			targetRadian = evadeRadian + static_cast<float>(std::numbers::pi);
		}
		break;

		case MoveState::Wander:
			// 徘徊時: 目標座標 (wanderTargetPos_) に向かう方向を向く
		{
			// 目標位置へのベクトル (wanderTargetPos_ - bossPosition)
			Vector3 toWanderTarget = Math::Subtract (wanderTargetPos_, bossPosition);

			// atan2(X成分, Z成分)で角度を求める
			float radian = std::atan2 (toWanderTarget.x, toWanderTarget.z);

			// 180度反転させて正面を向かせる (モデルの向きの調整)
			targetRadian = radian + static_cast<float>(std::numbers::pi);
		}
		break;

		case MoveState::Attack:
			// 攻撃時: 何もしない (現在の向きを維持)
			targetRadian = transform_.rotate.y;
			break;
		}
	} // ⭐️ if (Breath_->IsAttacking()) の終わり ⭐️


	// 5. ボスのY軸回転を設定
	// スムーズな回転が必要ならここに lerp 処理などを追加
	transform_.rotate.y = targetRadian;
}

void Boss::UpdateAnimation () {
	if (!centerStomp_->IsAttacking ()) {
		if (transform_.rotate.z >= 0.1f) {
			rotate_ = Rotate::left;
		}
		if (transform_.rotate.z <= -0.1f) {
			rotate_ = Rotate::right;
		}
		if (rotate_ == Rotate::left) {
			transform_.rotate.z -= 0.015f;
		}
		if (rotate_ == Rotate::right) {
			transform_.rotate.z += 0.015f;
		}
	}
}

std::vector<Collider*> Boss::GetAttackColliders () {
	std::vector<Collider*> colliders;

	// 1. CenterStompのColliderを追加 (単一のCollider)
	if (centerStomp_) {
		Collider* c = centerStomp_->GetCollider ();
		colliders.push_back (c);
	}

	// 2. FullScreenAttackのColliderを追加 (複数の弾)
	if (fullScreenAttack_) {
		std::vector<Collider*> bulletColliders = fullScreenAttack_->GetColliders ();
		colliders.insert (colliders.end (), bulletColliders.begin (), bulletColliders.end ());
	}

	// 3. BreathのColliderを追加 (複数の弾)
	if (Breath_) {
		std::vector<Collider*> breathColliders = Breath_->GetColliders ();
		colliders.insert (colliders.end (), breathColliders.begin (), breathColliders.end ());
	}

	return colliders;
}