#include "Bullet.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "Deltatime.h"

Bullet::Bullet(DxCommon* dxCommon, CameraOrganizer* camera, InputManager* input, LightManager* light) {
	camera_ = camera;
	model_ = std::make_unique<Model>(dxCommon, light);
}

Bullet::~Bullet() {

}

void Bullet::Initialize() {
	model_->SetModelData("monsterBall.obj");
	model_->SetTexture("bullet");
	model_->Initialize();

	speed_ = 30.0f;
	isActive_ = true;
	activeTimer_ = 3.0f;	// 秒単位
}

void Bullet::Update() {
	Move();
	Life();

	model_->SetPosition(transform_.translate);
	model_->Update(&camera_->GetCameraData());
}

void Bullet::Draw() {
	model_->Draw();
}

void Bullet::ImGui() {
	model_->ImGui("Bullet");
}

void Bullet::Move() {
	// 進向きを調整
	velocity_ = direction_ * speed_;

	if(isActive_) {
		transform_.translate += velocity_ * kDeltaTime;
	}
}

void Bullet::Life() {
	// 寿命が過ぎたらフラグを折る
	if(isActive_) { activeTimer_ -= kDeltaTime; }
	if(activeTimer_ < 0.0f) { isActive_ = false; }
}
