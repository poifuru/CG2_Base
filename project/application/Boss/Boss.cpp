#include "Boss.h"
#include "../Player/Player.h"
#include "MathFunction.h"
#include <imgui.h>

Boss::Boss(MagosuyaEngine* magosuya, Player* player) {
	magosuya_ = magosuya;
	player_ = player;
	model_ = std::make_unique<Model>(magosuya);
	magosuya_->LoadModelData("Resources/teapot", "teapot");
}

Boss::~Boss() {
	
}

void Boss::Initialize() {
	model_->SetModelData("teapot");
	model_->SetTexture("teapot");
	model_->Initialize();

	centerStomp_ = std::make_unique<CenterStomp>(magosuya_, this);
	centerStomp_->Initialize();

	fullScreenAttack_ = std::make_unique<FullScreenAttack>(magosuya_, this);
	fullScreenAttack_->Initialize();

	throwMinion_ = std::make_unique<ThrowMinion>(magosuya_, this);
	throwMinion_->Initialize();
}

void Boss::Update(Matrix4x4* m) {
	// 行動の更新
	UpdateMove();

	if (magosuya_->GetRawInput()->Trigger('1')) {
		centerStomp_->StartAttack();
	}
	if (magosuya_->GetRawInput()->Trigger('2')) {
		fullScreenAttack_->StartAttack();
	}
	if (magosuya_->GetRawInput()->Trigger('3')) {
		throwMinion_->StartAttack(100, 0.001f);
	}

	model_->Update(m);
	centerStomp_->Update(m);
	fullScreenAttack_->Update(m);
	throwMinion_->Update(m, player_->GetPosition());

	model_->SetTransform(transform_);
}

void Boss::Draw() {
	model_->Draw();
	centerStomp_->Draw();
	fullScreenAttack_->Draw();
	throwMinion_->Draw();
}

void Boss::ImGuiControl() {
#ifdef _DEBUG
	model_->ImGui ("boss");

	centerStomp_->ImGuiControl();
	fullScreenAttack_->ImGuiControl();
	throwMinion_->ImGuiControl();
#endif
}

bool Boss::IsAnyAttackActive() const {
	return centerStomp_->IsAttacking() || fullScreenAttack_->IsAttacking();
}

void Boss::UpdateMove() {
	if (IsAnyAttackActive()) {
		return;
	}

	// ブレス攻撃しているときの動き
	BreathMove();

	// 行動パターン
	NormalMove();
}

void Boss::BreathMove() {
	if (throwMinion_->IsAttacking()) {
		// 1. プレイヤーの座標を取得する
		Vector3 playerPosition = player_->GetPosition();
		Vector3 bossPosition = transform_.translate;

		// 2. プレイヤーへの方向ベクトルを計算する (プレイヤーの位置 - ボスの位置)
		// MathFunction.h に Subtract 関数が定義されていると仮定
		Vector3 toPlayerVector = Subtract(playerPosition, bossPosition);

		// 3. 方向ベクトルを正規化する
		float len = Length(toPlayerVector);
		if (len < 0.0001f) { return; }
		Vector3 direction = Normalize(toPlayerVector);

		// 4. 方向ベクトルに追尾速度を掛けて、今回のフレームの移動量を計算する
		// MathFunction.h に Multiply 関数が定義されていると仮定
		Vector3 moveAmount = Multiply(speed_, direction);

		// 5. ボスの位置を更新する
		transform_.translate.x += moveAmount.x;
		transform_.translate.y += moveAmount.y;
		transform_.translate.z += moveAmount.z;
	}
}

void Boss::NormalMove() {
	// 攻撃中でない場合、移動ステートを更新・実行
	UpdateMoveState(); // ステートを切り替えるロジック

	if (!throwMinion_->IsAttacking()) {
		switch (moveState_) {
		case MoveState::Wander:
			WanderMove();
			break;
		case MoveState::Follow:
			FollowMove();
			break;
		case MoveState::Evade:
			EvadeMove();
			break;
		default:
			break;
		}
	}
}

void Boss::WanderMove() {
	// 完全にランダムだと動きが不自然になるため、目標座標 (targetPos_) を設定し、そこへ向かうようにする
	static Vector3 targetPos_ = { 0.0f, 0.0f, 0.0f };
	static int updateCount = 0;
	// 💡 0:マップ中央, 1:マップ端, 2:ローカルランダム
	static int goalType = 0;

	// 一定フレームごと、または目標に近づいたら新しい目標座標を設定
	if (updateCount % 120 == 0 || Length(Subtract(targetPos_, transform_.translate)) < 1.0f) {

		// 💡 目標タイプをランダムに決定し、動きを読みにくくする
		goalType = rand() % 3;

		if (goalType == 0) {
			// マップ中央へ向かう
			targetPos_ = { 0.0f, 0.0f, 0.0f };
		} else if (goalType == 1) {
			// マップ端のランダムな位置へ向かう (例: X/Z -25から25の範囲をマップ端と仮定)
			targetPos_.x = static_cast<float>(rand() % 51 - 25);
			targetPos_.z = static_cast<float>(rand() % 51 - 25);
		} else {
			// 現在地付近のランダムな位置へ向かう (例: 現在地から±5の範囲)
			targetPos_.x = transform_.translate.x + static_cast<float>(rand() % 11 - 5);
			targetPos_.z = transform_.translate.z + static_cast<float>(rand() % 11 - 5);
		}

		updateCount = 0;
	}

	Vector3 toTarget = Subtract(targetPos_, transform_.translate);
	float len = Length(toTarget);
	if (len < 0.0001f) { return; }
	Vector3 direction = Normalize(toTarget);

	// 💡 目的のタイプによって速度に変化をつけても面白い (例: 端へ向かうときは少し速く)
	float currentSpeed = speed_ * 0.75f;
	if (goalType == 1) {
		currentSpeed = speed_ * 1.25f;
	}

	Vector3 moveAmount = Multiply(currentSpeed, direction);

	transform_.translate.x += moveAmount.x;
	transform_.translate.z += moveAmount.z;

	updateCount++;
}

void Boss::FollowMove() {
	// 1. プレイヤーの座標を取得する
	Vector3 playerPosition = player_->GetPosition();
	Vector3 bossPosition = transform_.translate;

	// 2. プレイヤーへの方向ベクトルを計算し、正規化する
	Vector3 toPlayerVector = Subtract(playerPosition, bossPosition);
	float len = Length(toPlayerVector);
	if (len < 0.0001f) { return; }
	Vector3 direction = Normalize(toPlayerVector);

	// 3. 方向ベクトルに追尾速度を掛けて、今回のフレームの移動量を計算する
	Vector3 moveAmount = Multiply(speed_, direction);

	// 4. ボスの位置を更新する
	transform_.translate.x += moveAmount.x;
	transform_.translate.z += moveAmount.z;
}

void Boss::EvadeMove() {
	// 1. プレイヤーの座標を取得する
	Vector3 playerPosition = player_->GetPosition();
	Vector3 bossPosition = transform_.translate;

	// 2. プレイヤーへの方向ベクトルを計算し、正規化する
	Vector3 toPlayerVector = Subtract(playerPosition, bossPosition);
	float len = Length(toPlayerVector);
	if (len < 0.0001f) { return; }
	Vector3 direction = Normalize(toPlayerVector); // プレイヤー方向

	// 3. 離脱なので、プレイヤー方向と逆向きに移動する (-direction)
	Vector3 awayDirection = Multiply(-1.0f, direction);

	// 4. 離脱速度を掛けて、今回のフレームの移動量を計算する
	Vector3 moveAmount = Multiply(speed_, awayDirection);

	// 5. ボスの位置を更新する
	transform_.translate.x += moveAmount.x;
	transform_.translate.z += moveAmount.z;
}

void Boss::UpdateMoveState() {
	Vector3 bossPosition = transform_.translate;
	Vector3 playerPosition = player_->GetPosition();
	// プレイヤーとの距離
	float distance = Length(Subtract(playerPosition, bossPosition));
	float distanceSq = distance * distance; // 距離の2乗

	moveTimer_++;

	switch (moveState_) {
	case MoveState::Wander:
		// プレイヤーが極端に遠すぎる/近すぎる場合は、緊急でステートを切り替えても良い
		if (distance > followDistance_ * 2.0f) { // 遠すぎたら、一旦近づく
			moveState_ = MoveState::Follow;
			moveTimer_ = 0;
		} else if (distance < evadeDistance_ / 2.0f) { // 近すぎたら、一旦離脱する
			moveState_ = MoveState::Evade;
			moveTimer_ = 0;
		}

		// 💡 自由徘徊時間が終了したら、戦術的移動に切り替える時間を大幅に延長 (maxMoveTime_ * 6 = 18秒)
		else if (moveTimer_ > maxMoveTime_ * 6) {
			// 自由徘徊時間が終了したら、次は戦術的移動（Follow/Evade）にランダムで切り替える
			if (rand() % 2 == 0) {
				moveState_ = MoveState::Follow;
			} else {
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