#include "Player.h"
#include <algorithm>
#include <numbers>
#include <imgui.h>
#include "InputManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "PlayScene.h"
#include "MathFunction.h"

//プレイヤーの大きさ
const float kPlayerWidth = 2.0f;
const float kPlayerHeight = 2.0f;

//デルタタイム
const float deltaTime = 1.0f / 60.0f;

//重力
const float kGravity = -0.01f;
// 落下速度の最大値（際限なく加速しないように）
const float kMaxFallSpeed = -0.5f;

Player::Player(DxCommon* dxCommon, CameraOrganizer* camera, InputManager* input, LightManager* light, MapChip* mapchip) {
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
	aabb_.min = { kPlayerWidth, -kPlayerHeight, -1.0f };
	aabb_.max = { kPlayerWidth,  kPlayerHeight,  1.0f };

	model_->SetPosition(mapchip_->GetMapChipPositionByIndex(2, 15));
}

void Player::Update() {
	//プレイヤーの挙動をここに
	FreeFall();

	CheckMapCollision(mapchip_);

	model_->Update(&camera_->GetCameraData());
}

void Player::Draw() {
	model_->Draw();
}

void Player::FreeFall() {
	// 重力を加算
	velocity_.y += kGravity;
	if(velocity_.y < kMaxFallSpeed) {
		velocity_.y = kMaxFallSpeed;
	}
}

void Player::ImGui() {
	model_->ImGui("player");
}