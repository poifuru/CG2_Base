#include "Bullet.h"
#include "Deltatime.h"
#include "imgui.h"
#include "MeshData.h"
#include "MathFunction.h"

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

	aabbSize_ = { 1.0f, 1.0f, 1.0f };

	speed_ = 30.0f;
	isActive_ = true;
	activeTimer_ = 3.0f;	// 秒単位

	hitbox_ = true;
	hitboxColor_ = { 1.0f, 1.0f, 1.0f, 1.0f };
}

void Bullet::Update() {
	Move();
	Life();

	model_->SetPosition(transform_.translate);
	model_->Update(&camera_->GetCameraData());
	// AABBの更新
	aabb_.min = transform_.translate - aabbSize_;
	aabb_.max = transform_.translate + aabbSize_;
}

void Bullet::Draw() {
	model_->Draw();

	if(hitbox_) {
		DrawHitbox(hitboxColor_);
	}
}

void Bullet::ImGui() {
	model_->ImGui("Bullet");
	ImGui::Checkbox("hitbox", &hitbox_);
}

void Bullet::Move() {
	// 進む向きを調整
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
