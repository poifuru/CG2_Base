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
	model_->SetScale({ 2.0f, 2.0f, 1.0f });

	positionOfset_ = { 0.0f, 0.0f, 40.0f };
	speed_ = 25.0f;
	localTranslate_ = { 0.0f, 0.0f, 0.0f };
}

void Reticle::Update() {
	Input();

	// 移動制限（ローカル座標系）
	localTranslate_.x = std::clamp(localTranslate_.x, -34.0f, 34.0f);
	localTranslate_.y = std::clamp(localTranslate_.y, -18.0f, 18.0f);

	if (railPath_) {
		Vector3 railPos = railPath_->GetPosition();
		Matrix4x4 railRot = railPath_->GetRotationMatrix();

		// レティクルの全ローカル座標（プレイヤー位置 + レティクル自体のズレ + 前方オフセット）
		Vector3 fullLocal = {
			playerLocalPos_.x + localTranslate_.x,
			playerLocalPos_.y + localTranslate_.y,
			positionOfset_.z
		};

		Vector3 rotatedLocal = Math::Transform(fullLocal, railRot);
		transform_.translate = Math::Add(railPos, rotatedLocal);
	} else {
		// レールが無いときのフォールバック（デバッグ用）
		transform_.translate.x += localTranslate_.x;
		transform_.translate.y += localTranslate_.y;
		transform_.translate.z = playerPos_.z + positionOfset_.z;
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
