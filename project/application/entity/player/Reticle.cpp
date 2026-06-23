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
	model_->IsLighting(LightReflectionModel::None);
	model_->SetAlpha(0.5f);
	model_->SetScale({ 0.7f, 0.7f, 0.7f });
	model_->SetColor({ 0.0f, 0.0f, 0.0f, 0.5f }); // 初期色を黒にする

	positionOfset_ = { 0.0f, 0.0f, 20.0f };
	speed_ = 25.0f;
	localTranslate_ = { 0.0f, 0.0f, 0.0f };
	isLockOn_ = false;
}

void Reticle::Update() {
	// 矢印キーでの手動移動は行わない
	// Input();

	if (camera_) {
		// カメラの正面30.0fの位置にレティクルを配置
		Matrix4x4 camWorld = camera_->GetCameraData().world;
		Vector3 camForward = { camWorld.m[2][0], camWorld.m[2][1], camWorld.m[2][2] };
		Vector3 cameraPos = camera_->GetCameraData().transform.translate;

		transform_.translate = Math::Add(cameraPos, Math::Multiply(30.0f, camForward));

		// レティクルモデルの回転をカメラの回転と同期（ビルボード化）
		model_->SetRotate(camera_->GetCameraData().transform.rotate);
	}

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
		localTranslate_.x += moveDir.x * speed_ * kDeltaTime;
		localTranslate_.y += moveDir.y * speed_ * kDeltaTime;
	}
}

void Reticle::SetLockOn(bool lock) {
	isLockOn_ = lock;
	if (isLockOn_) {
		model_->SetColor({ 1.0f, 0.0f, 0.0f, 0.5f }); // 赤
	} else {
		model_->SetColor({ 0.0f, 0.0f, 0.0f, 0.5f }); // 黒
	}
}
