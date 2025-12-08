#include "PlayerState.h"
#include "Player.h"

void PlayerChargeState::Initialize() {
    if (!player_) return;

    // チャージしているときはスタミナを回復させるかどうか迷う

    // 移動速度
    player_->SetSpeedMultiplier(0.35f);

    // チャージタイマーリセット
    chargeTimer_ = 0.0f;
    currentChargeLevel_ = 0;

    // チャージ開始のアニメーションやエフェクト再生
    // player_->PlayChargeStartAnimation(); 
}

void PlayerChargeState::Update() {

    auto& move = player_->Move();
    if (player_->engine_->GetRawInput()->Push('W')){
        move.z = 1.0f;
    }
    if (player_->engine_->GetRawInput()->Push('S')){
        move.z = -1.0f;
    }
    if (player_->engine_->GetRawInput()->Push('A')){
        move.x = -1.0f;
    }
    if (player_->engine_->GetRawInput()->Push('D')){
        move.x = 1.0f;
    }

    const float deltaTime = 1.0f / 60.0f;

    // 1. 入力チェック: ボタンが離されたか？
    if (player_->input_->GetRawInput()->Push('J') == false) {
        // ボタンが離されたら、チャージ解放状態へ遷移
        player_->ChangeState(new PlayerChargeReleaseState(currentChargeLevel_));
        return;
    }

    // 2. タイマー更新
    chargeTimer_ += deltaTime;

    // 3. チャージレベルの判定
    // 例: 0.5秒で Level 1, 1.5秒で Level 2 (MAX)
    if (chargeTimer_ < 0.5f) {
        currentChargeLevel_ = 1; // Level 1 チャージ

        // [Level1は速度の変更は特になし]
        // 移動速度(初期値)
        player_->SetSpeedMultiplier(0.35f);
    }
    else if (chargeTimer_ < 1.5f) {
        currentChargeLevel_ = 2; // Level 2 チャージ

        // 移動速度の変更
        player_->SetSpeedMultiplier(0.20f);

        // エフェクトの変更
    }
    else {
        currentChargeLevel_ = 3; // Level 3 チャージ (MAX)

        // 移動速度の変更
        player_->SetSpeedMultiplier(0.10f);

        // 最大チャージ時のエフェクト
    }

    if (!player_->IsOnGround()) {
        // 空中判定（崖から落ちた場合など）
        player_->ChangeState(new PlayerFallState());
        return;
    }

    // 回避でキャンセル出来るようにしてもいい
    /*if (もし回避行動をしたら) {
        回避のStateに遷移させる
        return;
    }*/

#ifdef _DEBUG
    ImGui::Begin("Player");
    ImGui::DragInt("ChargeLevel", &currentChargeLevel_);
    ImGui::End();
#endif//_DEBUG
}

void PlayerChargeState::Exit() {
    if (!player_) return;

    // 移動速度 [ 元に戻す ]
    player_->SetSpeedMultiplier(1.0f);

    // スタミナ回復ブロックの解除
    //player_->UnblockStaminaRecovery();

    // チャージエフェクトを停止
    // player_->StopChargeEffect();
}