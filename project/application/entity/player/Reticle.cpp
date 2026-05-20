#include <algorithm>
#include "Reticle.h"
#include "DeltaTime.h"

Reticle::Reticle(DxCommon* dxCommon, CameraOrganizer* camera, InputManager* input, LightManager* light) {
	model_ = std::make_unique<Model>(dxCommon, light);

	input_ = input;
	camera_ = camera;
}

Reticle::~Reticle() {

}

void Reticle::Initialize() {
	model_->SetModelData("reticle.obj");
	model_->SetTexture("reticle");
	model_->Initialize();
	model_->SetDepthEnable(false);
	model_->IsLighting(FALSE);
	model_->SetColor(Vector4{ 1.0f, 1.0f, 0.0f, 0.5f });
	model_->SetScale({ 2.0f, 2.0f, 1.0f });

	positionOfset_ = { 0.0f, 0.0f, 40.0f };
	speed_ = 25.0f;
}

void Reticle::Update() {
	Input();
	transform_.translate.z = playerPos_.z + positionOfset_.z;

	transform_.translate.x = std::clamp(transform_.translate.x, -34.0f, 34.0f);
	transform_.translate.y = std::clamp(transform_.translate.y, -18.0f, 18.0f);

	model_->SetPosition(transform_.translate);
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
		transform_.translate.x += moveDir.x * speed_ * kDeltaTime;
		transform_.translate.y += moveDir.y * speed_ * kDeltaTime;
	}
}
