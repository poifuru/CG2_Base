#include "Player.h"
#include <algorithm>
#include <numbers>
#include <imgui.h>
#include "InputManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "PlayScene.h"
#include "MathFunction.h"

//プレイヤーの大きさ(半径)
const float kPlayerWidth = 0.9f;
const float kPlayerHeight = 0.9f;

//デルタタイム
const float deltaTime = 1.0f / 60.0f;

//重力
const float kGravity = -0.01f;
// 落下速度の最大値（際限なく加速しないように）
const float kMaxFallSpeed = -0.5f;

Player::Player(DxCommon* dxCommon, CameraOrganizer* camera, InputManager* input, LightManager* light, MapChip* mapchip) {
	isDoubleJump_ = false;
	transform_ = {};
	
	camera_ = camera;
	input_ = input;
	mapchip_ = mapchip;
	model_ = std::make_unique<Model>(dxCommon, light);
	weapon_ = std::make_unique<Weapon>(dxCommon, light);
}

Player::~Player() {

}

void Player::Initialize() {
	model_->Initialize();
	model_->SetModelData("player.obj");
	model_->SetTexture("player");
	aabb_.min = { -kPlayerWidth, -kPlayerHeight, 0.0f };
	aabb_.max = { kPlayerWidth, kPlayerHeight, 0.0f };

	weapon_->Initialize();

	transform_.translate = mapchip_->GetMapChipPositionByIndex(3, 15);
}

void Player::Update() {
	//プレイヤーの挙動をここに
	ProcessDash();
	Input();
	FreeFall();
	WallKickTimer();

	//武器の更新処理
	weapon_->Update(transform_.translate, faceDirection_, updownDirection_, isGrounded_, &camera_->GetCameraData());
	
	CheckMapCollision(mapchip_);
	EntityState();

	model_->SetPosition(transform_.translate);
	model_->Update(&camera_->GetCameraData());
}

void Player::Draw() {
	model_->Draw();
	weapon_->Draw();
}

void Player::Input() {
	//加速度の強さ
	const float kAccelerationPower = 0.04f;
	//スピード
	float speed_ = 0.15f;

	//ジャンプ
	float kJumpPower = 0.3f;
	const float kKickPushPower = 0.25f; //壁から離れる方向への力

	//上下の向き
	if(input_->GetRawInput()->Push('W')) updownDirection_ = 1.0f;
	if(input_->GetRawInput()->Push('S')) updownDirection_ = -1.0f;

	if(!isDashing_) {
		//左右移動
		if(wallKickTimer_ <= 0) {
			velocity_.x = 0.0f;
			if(input_->GetRawInput()->Push('A')) {
				velocity_.x = -speed_;
				faceDirection_ = -1.0f; // 左を向く
			}
			if(input_->GetRawInput()->Push('D')) {
				velocity_.x = speed_;
				faceDirection_ = 1.0f;  // 右を向く
			}
		}

		if(isGrounded_ && input_->GetRawInput()->Trigger(VK_SPACE)) {
			velocity_.y = kJumpPower;
		}
		//壁キック（空中且つ壁に触れている）
		else if(!isGrounded_ && input_->GetRawInput()->Trigger(VK_SPACE)) {
			//右の壁に触れていて、かつ右キーDを押している
			if(isTouchingWallRight_ && input_->GetRawInput()->Push('D')) {
				velocity_.y = kJumpPower;
				velocity_.x = -kKickPushPower; //左に蹴り出す
				isDoubleJump_ = false;
				wallKickTimer_ = 10;           //入力禁止
			}
			//左の壁に触れていて、かつ左キーAを押している
			else if(isTouchingWallLeft_ && input_->GetRawInput()->Push('A')) {
				velocity_.y = kJumpPower;
				velocity_.x = kKickPushPower;  //右に蹴り出す
				isDoubleJump_ = false;
				wallKickTimer_ = 10;           //入力禁止
			}
			isDashing_ = false;
		}
		//二段ジャンプ
		if(!isTouchingWallLeft_ && !isTouchingWallRight_ && !isGrounded_ && !isDoubleJump_ && input_->GetRawInput()->Trigger(VK_SPACE)) {
			velocity_.y = kJumpPower * 0.8f;
			isDoubleJump_ = true;
		}
	}

	if(input_->GetRawInput()->TriggerMouse(0)) {
		weapon_->Attack();
	}
}

void Player::FreeFall() {
	if(!isDashing_) {
		//壁ずり判定
		bool isWallSliding = false;
		const float kWallSlideSpeed = -0.15f;

		//右壁に触れていて、Dキー右を押している
		if(isTouchingWallRight_ && input_->GetRawInput()->Push('D')) {
			isWallSliding = true;
		}
		//左壁に触れていて、Aキー左を押している
		else if(isTouchingWallLeft_ && input_->GetRawInput()->Push('A')) {
			isWallSliding = true;
		}

		//重力の適用
		velocity_.y += kGravity;

		//壁ずり中の速度制限
		if(isWallSliding && velocity_.y < kWallSlideSpeed) {
			velocity_.y = kWallSlideSpeed;
		}
	}
}

void Player::WallKickTimer() {
	// タイマーを減らす
	if(wallKickTimer_ > 0) {
		wallKickTimer_--;
	}
}

void Player::ProcessDash() {
	// クールタイムのカウントダウン
	if(dashCooldown_ > 0) {
		dashCooldown_--;
	}

	// ダッシュの開始判定 (ダッシュ中ではなく、クールタイムが終わっているとき)
	if(!isDashing_ && dashCooldown_ <= 0) {
		if(input_->GetRawInput()->TriggerMouse(1)) {
			isDashing_ = true;
			dashTimer_ = 10;    // ダッシュの持続フレーム数
			dashCooldown_ = 30; // 次に出せるまでの時間

			// 向いている方向にダッシュ（A/D入力があればそれを優先）
			if(input_->GetRawInput()->Push('A')) {
				dashDirection_ = -1.0f;
			}
			else if(input_->GetRawInput()->Push('D')) {
				dashDirection_ = 1.0f;
			}
			else {
				// 入力がない場合は現在の速度の向きを参照
				dashDirection_ = (velocity_.x >= 0.0f) ? 1.0f : -1.0f;
			}
		}
	}

	// ダッシュ実行中の処理
	if(isDashing_) {
		const float kDashSpeed = 0.8f; // ダッシュの速さ

		velocity_.x = dashDirection_ * kDashSpeed;
		velocity_.y = 0.0f; // ダッシュ中は重力無視

		dashTimer_--;
		if(dashTimer_ <= 0) {
			isDashing_ = false;
			// 終わった瞬間に慣性を少し残すとスムーズでやんす
			velocity_.x *= 0.5f;
		}
	}
}

void Player::EntityState() {
	if(isGrounded_) {
		isDoubleJump_ = false;
	}
}

void Player::ImGui() {
#ifdef USEIMGUI
	model_->ImGui("player");
	ImGui::Text("isGrounded : %d", isGrounded_);
	ImGui::Text("isDoubleJump : %d", isDoubleJump_);
	ImGui::Text("isWallTouchLeft : %d", isTouchingWallLeft_);
	ImGui::Text("isWallTouchRight : %d", isTouchingWallRight_);
	ImGui::Text("isDashing : %d", isDashing_);
	weapon_->ImGui();
#endif
}