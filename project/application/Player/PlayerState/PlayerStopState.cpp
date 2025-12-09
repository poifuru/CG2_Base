#include "PlayerState.h"
#include "Player.h"

void PlayerStopState::Initialize() {

}

void PlayerStopState::Update() {

	bool isMove = false;
	if (player_->input_->GetRawInput()->Push('W')) { isMove = true; }
	else if (player_->input_->GetRawInput()->Push('S')) { isMove = true; }
	else if (player_->input_->GetRawInput()->Push('A')) { isMove = true; }
	else if (player_->input_->GetRawInput()->Push('D')) { isMove = true; }
	if (player_->input_->GetGamePad()->IsConection()) {
		Vector2 gamepad;
		gamepad = player_->input_->GetGamePad()->GetStick(LR::Left);
		if (gamepad.x != 0.0f || gamepad.y != 0.0f) {
			// どちらか片方動いて入れば
			isMove = true;
		}
	}

	bool has_stamina = player_->GetStamina() > 0.0f;

	if (has_stamina == false) {
		// スタミナ切れ
		player_->ChangeState(new PlayerExhaustedState());
		return;
	}

	if (isMove) {
		player_->ChangeState(new PlayerWalkState());
		return;
	}

	//if (InputManager::IsDash()) {
	//	// 走る
	//	player_->ChangeState(new PlayerDashState());
	//	return;
	//}

	if (player_->IsAttack()) {
		// 攻撃
		player_->ChangeState(new PlayerAttackState());
		return;
	}
}