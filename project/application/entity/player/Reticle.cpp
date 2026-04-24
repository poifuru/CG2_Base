#include "Reticle.h"
#include "DeltaTime.h"

Reticle::Reticle(DxCommon* dxCommon, CameraOrganizer* camera, InputManager* input, LightManager* light) {
	model_ = std::make_unique<Model>(dxCommon, light);
	model_->IsLighting(LightReflectionModel::None);
	model_->SetAlpha(0.5f);

	input_ = input;
	camera_ = camera;
}

Reticle::~Reticle() {

}

void Reticle::Initialize() {
	model_->SetModelData("reticle.obj");
	model_->SetTexture("reticle");
	model_->Initialize();

	positionOfset_ = { 0.0f, 0.0f, 20.0f };
	speed_ = 3.0f;
}

void Reticle::Update() {
	Input();
	transform_.translate = playerPos_ + positionOfset_;

	model_->Update(&camera_->GetCameraData());
}

void Reticle::Draw() {
	model_->Draw();
}

void Reticle::ImGui() {

}

void Reticle::Input() {
	// フレーム内の入力を方向として蓄積
	Vector2 moveDir = { 0.0f, 0.0f };

	if(input_->GetRawInput()->Push(VK_UP)) { moveDir.y += 1.0f; }
	if(input_->GetRawInput()->Push(VK_DOWN)) { moveDir.y -= 1.0f; }
	if(input_->GetRawInput()->Push(VK_LEFT)) { moveDir.x -= 1.0f; }
	if(input_->GetRawInput()->Push(VK_RIGHT)) { moveDir.x += 1.0f; }

	// 入力があった場合に処理する
	if(moveDir.x != 0.0f || moveDir.y != 0.0f) {
		// ベクトルの長さを計算
		float length = std::sqrt(moveDir.x * moveDir.x + moveDir.y * moveDir.y);

		// 正規化
		moveDir.x /= length;
		moveDir.y /= length;

		// 実際に速度、デルタタイムを掛ける
		velocity_.x += moveDir.x * speed_ * kDeltaTime;
		velocity_.y += moveDir.y * speed_ * kDeltaTime;
	}
}
