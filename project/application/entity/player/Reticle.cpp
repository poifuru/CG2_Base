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
	model_->SetScale({ 1.5f, 1.5f, 1.0f });
	model_->SetColor({ 0.0f, 0.0f, 0.0f, 0.5f }); // 初期色を黒にする

	positionOfset_ = { 0.0f, 0.0f, 20.0f };
	speed_ = 25.0f;
	localTranslate_ = { 0.0f, 0.0f, 0.0f };
	isLockOn_ = false;
}

void Reticle::Update() {
	Input();

	if (camera_) {
		// 1. クランプ前の予測ワールド座標を計算する
		Vector3 worldPos = transform_.translate;
		if (railPath_) {
			Vector3 railPos = railPath_->GetPosition();
			Matrix4x4 railRot = railPath_->GetRotationMatrix();
			Vector3 fullLocal = {
				playerLocalPos_.x + localTranslate_.x,
				playerLocalPos_.y + localTranslate_.y,
				positionOfset_.z
			};
			Vector3 rotatedLocal = Math::Transform(fullLocal, railRot);
			worldPos = Math::Add(railPos, rotatedLocal);
		}

		// 2. カメラのビュー空間に変換する
		Matrix4x4 viewMat = camera_->GetCameraData().view;
		Vector3 viewPos = Math::Transform(worldPos, viewMat);

		// 3. ビュー空間のZ軸（カメラからの距離）を基準に、制限限界値を計算
		float distanceToCamera = viewPos.z;
		if (distanceToCamera <= 0.0f) {
			distanceToCamera = 50.0f;
		}

		float fovY = 0.45f;
		float aspect = 1280.0f / 720.0f;

		float halfHeight = std::tan(fovY * 0.5f) * distanceToCamera;
		float halfWidth = halfHeight * aspect;

		float marginX = 1.0f;
		float marginY = 1.0f;

		float limitX = (std::max)(0.0f, halfWidth - marginX);
		float limitY = (std::max)(0.0f, halfHeight - marginY);

		// 4. ビュー空間上でクランプ
		viewPos.x = std::clamp(viewPos.x, -limitX, limitX);
		viewPos.y = std::clamp(viewPos.y, -limitY, limitY);

		// 5. ワールド座標に戻す
		Matrix4x4 cameraWorld = camera_->GetCameraData().world;
		worldPos = Math::Transform(viewPos, cameraWorld);

		// 6. レールのローカル座標 (localTranslate_) に逆変換して戻す
		if (railPath_) {
			Vector3 railPos = railPath_->GetPosition();
			Matrix4x4 railRot = railPath_->GetRotationMatrix();
			Matrix4x4 invRailRot = Math::Inverse(railRot);

			Vector3 diff = Math::Subtract(worldPos, railPos);
			Vector3 localResult = Math::Transform(diff, invRailRot);
			localTranslate_.x = localResult.x - playerLocalPos_.x;
			localTranslate_.y = localResult.y - playerLocalPos_.y;
		} else {
			localTranslate_.x = viewPos.x - playerLocalPos_.x;
			localTranslate_.y = viewPos.y - playerLocalPos_.y;
		}
	}

	if (railPath_) {
		Vector3 railPos = railPath_->GetPosition();
		Matrix4x4 railRot = railPath_->GetRotationMatrix();

		Vector3 fullLocal = {
			playerLocalPos_.x + localTranslate_.x,
			playerLocalPos_.y + localTranslate_.y,
			positionOfset_.z
		};

		Vector3 rotatedLocal = Math::Transform(fullLocal, railRot);
		transform_.translate = Math::Add(railPos, rotatedLocal);
	} else {
		transform_.translate.x += localTranslate_.x;
		transform_.translate.y += localTranslate_.y;
		transform_.translate.z = playerPos_.z + positionOfset_.z;
	}

	// レティクルモデルの回転をカメラの回転と同期（ビルボード化）
	if (camera_) {
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
