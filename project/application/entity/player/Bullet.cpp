#include "Bullet.h"
#include "Deltatime.h"
#include "imgui.h"
#include "Mesh.h"
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

	if(hitbox_) {
		auto x = transform_.translate.x;
		auto y = transform_.translate.y;
		auto z = transform_.translate.z;
		auto scaleX = transform_.scale.x;
		auto scaleY = transform_.scale.y;
		auto scaleZ = transform_.scale.z;

		// Box上の頂点8つを定義
		Vector3 leftUpFront = { x - scaleX, y + scaleY, z - scaleZ };		// 左上手前
		Vector3 leftDownFront = { x - scaleX, y - scaleY, z - scaleZ };		//　左下手前
		Vector3 rightDownFront = { x + scaleX, y - scaleY, z - scaleZ };	//　右下手前
		Vector3 rightUpFront = { x + scaleX, y + scaleY, z - scaleZ };		//　右上手前
		Vector3 leftUpBack = { x - scaleX, y + scaleY, z + scaleZ };		//　左上奥
		Vector3 leftDownBack = { x - scaleX, y - scaleY, z + scaleZ };		//　左下奥
		Vector3 rightDownBack = { x + scaleX, y - scaleY, z + scaleZ };		//　右下奥
		Vector3 rightUpBack = { x + scaleX, y + scaleY, z + scaleZ };		//　右上奥

		Mesh::DrawLine(
			leftUpFront.x, leftUpFront.y, leftUpFront.z, 
		)
	}
}

void Bullet::ImGui() {
	model_->ImGui("Bullet");
	ImGui::Checkbox("hitbox", &hitbox_);
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
