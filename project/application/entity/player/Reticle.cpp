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
		// カメラとレティクルベース位置（レール位置）の座標を取得
		Vector3 cameraPos = camera_->GetCameraData().transform.translate;
		Vector3 basePos = transform_.translate;
		if (railPath_) {
			basePos = railPath_->GetPosition();
		}

		// カメラからの実際の距離を計算
		float distanceToCamera = basePos.z - cameraPos.z + positionOfset_.z;
		if (distanceToCamera <= 0.0f) {
			distanceToCamera = 40.0f;
		}

		// 画角とアスペクト比から画面の半分サイズを計算
		float fovY = 0.45f;
		float aspect = 1280.0f / 720.0f;

		float halfHeight = std::tan(fovY * 0.5f) * distanceToCamera;
		float halfWidth = halfHeight * aspect;

		// レティクル用のマージンを設定（画面ギリギリまで狙えるように少し小さめ）
		float marginX = 1.0f;
		float marginY = 1.0f;

		// 画面全体の左右制限幅 (X軸)
		float limitX = (std::max)(0.0f, halfWidth - marginX);

		// カメラの高さ（Y軸のオフセット）のズレを計算して上下の限界値をシフト
		float cameraHeightOffset = cameraPos.y - basePos.y;
		float limitTop    =  halfHeight - marginY + cameraHeightOffset;
		float limitBottom = -halfHeight + marginY + cameraHeightOffset;

		if (limitTop < limitBottom) {
			limitTop = marginY;
			limitBottom = -marginY;
		}

		// -------------------------------------------------------------
		// 【ココが修正のキモ！】
		// 入力によって移動しようとしている「最終的な画面ローカル座標」を一度計算する
		float finalLocalX = playerLocalPos_.x + localTranslate_.x;
		float finalLocalY = playerLocalPos_.y + localTranslate_.y;

		// その最終座標が、画面の限界（limitX, limitBottom, limitTop）を超えないようにクランプ！
		finalLocalX = std::clamp(finalLocalX, -limitX, limitX);
		finalLocalY = std::clamp(finalLocalY, limitBottom, limitTop);

		// クランプした後の最終座標から、プレイヤーの座標を引いて、
		// レティクル独自の純粋なズレ量（localTranslate_）に逆算して戻す！
		localTranslate_.x = finalLocalX - playerLocalPos_.x;
		localTranslate_.y = finalLocalY - playerLocalPos_.y;
		// -------------------------------------------------------------
	}
	// =================================================================

	if (railPath_) {
		Vector3 railPos = railPath_->GetPosition();
		Matrix4x4 railRot = railPath_->GetRotationMatrix();

		// ここは今までの計算のままでOK（上記で localTranslate_ が正しく補正されたため）
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

void Reticle::SetLockOn(bool lock) {
	isLockOn_ = lock;
	if (isLockOn_) {
		model_->SetColor({ 1.0f, 0.0f, 0.0f, 0.5f }); // 赤
	} else {
		model_->SetColor({ 0.0f, 0.0f, 0.0f, 0.5f }); // 黒
	}
}
