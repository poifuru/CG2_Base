#include "Player.h"
#include <algorithm>
#include <numbers>
#include <imgui.h>
#include "InputManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "PlayScene.h"
#include "MathFunction.h"
#include "DeltaTime.h"

// キー入力が無いときに速度を減衰させる定数
static const float kAttenuationRate = 0.95f;

Player::Player(DxCommon* dxCommon, CameraOrganizer* camera, InputManager* input, LightManager* light) {
	transform_ = {};
	
	camera_ = camera;
	input_ = input;
	model_ = std::make_unique<Model>(dxCommon, light);

	TextureManager::GetInstance()->LoadTexture("Resources/player/player.png", "player");
	ModelManager::GetInstance()->LoadModelData("Resources/player", "player.obj");
}

Player::~Player() {

}

void Player::Initialize() {
	model_->SetModelData("player.obj");
	model_->SetTexture("player");
	model_->Initialize();

	// 固有の数値
	speed_ = 3.0f;
	velocity_ = { 0.0f, 0.0f, 0.1f };
}

void Player::Update() {
	//プレイヤーの挙動をここに
	Input();
	Move();

	//実際にモデルを動かす
	model_->SetPosition(transform_.translate);
	model_->Update(&camera_->GetCameraData());
}

void Player::Draw() {
	model_->Draw();
}

void Player::Input() {
	acceleration_.x = 0.0f;
	acceleration_.y = 0.0f;

	if(InputManager::GetInstance()->GetRawInput()->Push('W')) {
		acceleration_.y = speed_ * kDeltaTime;
		velocity_.y += acceleration_.y;
	}
	if(InputManager::GetInstance()->GetRawInput()->Push('S')) {
		acceleration_.y = -speed_ * kDeltaTime;
		velocity_.y += acceleration_.y;
	}
	if(InputManager::GetInstance()->GetRawInput()->Push('A')) {
		acceleration_.x = -speed_ * kDeltaTime;
		velocity_.x += acceleration_.x;
	}
	if(InputManager::GetInstance()->GetRawInput()->Push('D')) {
		acceleration_.x = speed_ * kDeltaTime;
		velocity_.x += acceleration_.x;
	}
}

void
 Player::Move() {
	// 速度に減衰率をかけ続ける
	velocity_.x *= kAttenuationRate;
	velocity_.y *= kAttenuationRate;

	// 速度が上限、下限に触れていないかチェック
	const float maxSpeed = 5.0f;
	velocity_.x = std::clamp(velocity_.x, -maxSpeed, maxSpeed);
	velocity_.y = std::clamp(velocity_.y, -maxSpeed, maxSpeed);

	// 減衰して速度が一定以下になったら0とみなす
	// 速度の最低値
	const float minSpeed = 0.02f;

	if(std::abs(velocity_.x) < minSpeed) {
		velocity_.x = 0.0f;
	}
	if(std::abs(velocity_.y) < minSpeed) {
		velocity_.y = 0.0f;
	}

	// 実際の移動処理
	transform_.translate += velocity_;

	// 移動制限
	transform_.translate.x = std::clamp(transform_.translate.x, -18.0f, 18.0f);
	transform_.translate.y = std::clamp(transform_.translate.y, -10.0f, 10.0f);
}

void Player::ImGui() {
#ifdef USEIMGUI
	ImGui::Begin("Player");
	ImGui::DragFloat3("Position", &transform_.translate.x, 0.01f);
	ImGui::DragFloat3("Velocity", &velocity_.x, 0.01f);
	ImGui::DragFloat3("Acceleration", &acceleration_.x, 0.01f);
	ImGui::End();
#endif
}