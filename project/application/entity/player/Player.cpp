#include "Player.h"
#include <algorithm>
#include <numbers>
#include <imgui.h>
#include "InputManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "PlayScene.h"
#include "MathFunction.h"

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

	speed_ = 0.001f;
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
		acceleration_.y = speed_;
	}
	if(InputManager::GetInstance()->GetRawInput()->Push('S')) {
		acceleration_.y = -speed_;
	}
	if(InputManager::GetInstance()->GetRawInput()->Push('A')) {
		acceleration_.x = -speed_;
	}
	if(InputManager::GetInstance()->GetRawInput()->Push('D')) {
		acceleration_.x = speed_;
	}

	velocity_.x += acceleration_.x;
	velocity_.y += acceleration_.y;
}

void Player::Move() {
	// 画面内を左右に移動
	if(transform_.translate.x <= 20.0f || transform_.translate.x >= 0.0f) {
		transform_.translate.x += velocity_.x;
	}
	if(transform_.translate.y <= 15.0f || transform_.translate.y >= 0.0f) {
		transform_.translate.y += velocity_.y;
	}

	transform_.translate.z += velocity_.z;
}

void Player::ImGui() {
#ifdef USEIMGUI
#endif
}