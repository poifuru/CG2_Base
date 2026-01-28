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
}

Player::~Player() {

}

void Player::Initialize() {
	model_->Initialize();
	model_->SetModelData("player.obj");
	model_->SetTexture("player");
	// AABBは中心(0,0,0)からの相対サイズで設定するでやんす！
	aabb_.min = { -kPlayerWidth, -kPlayerHeight, 0.0f };
	aabb_.max = { kPlayerWidth, kPlayerHeight, 0.0f };

	transform_.translate = mapchip_->GetMapChipPositionByIndex(3, 15);
}

void Player::Update() {
	//プレイヤーの挙動をここに
	FreeFall();
	Input();
	
	//加速度適用
	Acceleration();

	CheckMapCollision(mapchip_);
	EntityState();

	model_->SetPosition(transform_.translate);
	model_->Update(&camera_->GetCameraData());
}

void Player::Draw() {
	model_->Draw();
}

void Player::Input() {
	// 加速度の強さ
	const float kAccelerationPower = 0.04f;
	//スピード
	float speed_ = 0.2f;

	//左右移動
	velocity_.x = 0.0f;
	if(input_->GetRawInput()->Push('A')) {
		velocity_.x = -speed_;
	}
	if(input_->GetRawInput()->Push('D')) {
		velocity_.x = speed_;
	}

	//ジャンプ
	float kJumpPower = 0.3f;
	const float kKickPushPower = 0.4f; // 壁から離れる方向への力

	if(isGrounded_ && input_->GetRawInput()->Trigger(VK_SPACE)) {
		velocity_.y = kJumpPower;
	}
	// ★ 壁キック（空中且つ壁に触れている）
	else if(!isGrounded_ && input_->GetRawInput()->Trigger(VK_SPACE)) {
		// 右の壁に触れていて、かつ右キー（D）を押している
		if(isTouchingWallRight_ && input_->GetRawInput()->Push('D')) {
			velocity_.y = kJumpPower;
			velocity_.x = -kKickPushPower; // 左に蹴り出す
		}
		// 左の壁に触れていて、かつ左キー（A）を押している
		else if(isTouchingWallLeft_ && input_->GetRawInput()->Push('A')) {
			velocity_.y = kJumpPower;
			velocity_.x = kKickPushPower;  // 右に蹴り出す
		}
	}
	if(!isTouchingWallLeft_ && !isTouchingWallRight_ && !isGrounded_ && !isDoubleJump_ && input_->GetRawInput()->Trigger(VK_SPACE)) {
		velocity_.y = kJumpPower * 0.8f;
		isDoubleJump_ = true;
	}

	//ダッシュ
	if(input_->GetRawInput()->TriggerMouse(1)) {

	}
}

void Player::FreeFall() {
	// 壁ずり判定
	bool isWallSliding = false;
	const float kWallSlideSpeed = -0.05f;

	// 右壁に触れていて、Dキー（右）を押している
	if(isTouchingWallRight_ && input_->GetRawInput()->Push('D')) {
		isWallSliding = true;
	}
	// 左壁に触れていて、Aキー（左）を押している
	else if(isTouchingWallLeft_ && input_->GetRawInput()->Push('A')) {
		isWallSliding = true;
	}

	// 重力の適用（加速度に加算）
	acceleration_.y += kGravity;

	// 壁ずり中の速度制限
	if(isWallSliding && velocity_.y < kWallSlideSpeed) {
		velocity_.y = kWallSlideSpeed;
		acceleration_.y = 0.0f; // 重力による加速を止める
	}
}

void Player::Acceleration() {
	// 2. 加速度を速度に加算 (v += a)
	velocity_.x += acceleration_.x;
	velocity_.y += acceleration_.y;

	// 3. 摩擦の適用 (慣性で止まるように)
	// 地上にいるときは強く、空中にいるときは少し弱くすると操作感が良くなるでやんす
	float currentFriction = isGrounded_ ? friction_ : 0.95f;
	velocity_.x *= currentFriction;

	// 4. 最大速度の制限
	const float kMaxHorizontalSpeed = 0.5f;
	velocity_.x = std::clamp(velocity_.x, -kMaxHorizontalSpeed, kMaxHorizontalSpeed);
}

void Player::EntityState() {
	if(isGrounded_) {
		isDoubleJump_ = false;
	}
}

void Player::ImGui() {
	model_->ImGui("player");
	ImGui::Text("isGrounded : %d", isGrounded_);
	ImGui::Text("isDoubleJump : %d", isDoubleJump_);
	ImGui::Text("isWallTouchLeft : %d", isTouchingWallLeft_);
	ImGui::Text("isWallTouchRight : %d", isTouchingWallRight_);
}