#include "DebugCamera.h"

DebugCamera::DebugCamera() {
	transform_ = {};
	worldMatrix_ = {};
	viewMatrix_ = {};
	projectionMatrix_ = {};

	forward_ = {};
	right_ = {};
	speed_ = 0.0f;
	move_ = {};

	sensitivity_ = 0.001f;
	pitchOver_ = 1.5708f;
}

void DebugCamera::Initialize() {
	transform_ = {
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, -10.0f},
	};
	worldMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	viewMatrix_ = Inverse(worldMatrix_);
	projectionMatrix_ = MakePerspectiveFOVMatrix(0.45f, float(1280) / float(720), 0.1f, 100.0f);

	speed_ = 0.1f;
}

void DebugCamera::Updata(HWND hwnd, HRESULT hr, ComPtr<IDirectInputDevice8W> keyboard, BYTE key[], ComPtr<IDirectInputDevice8W> mouse, MouseInput* mouseInput) {
	//=======入力処理の準備=======//
	//キーボードの状態を取得
	keyboard->GetDeviceState(sizeof(key), key);
	//マウスの状態を取得
	DIMOUSESTATE mouseState = {};
	hr = mouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState);

	//=======更新処理=======//
	//カメラの前後左右の移動
	forward_ = {
	worldMatrix_.m[2][0], // z軸のx成分
	worldMatrix_.m[2][1], // z軸のy成分
	worldMatrix_.m[2][2]  // z軸のz成分
	};
	forward_ = Normalize(forward_);

	right_ = {
	worldMatrix_.m[0][0], // x軸のx成分
	worldMatrix_.m[0][1], // x軸のy成分
	worldMatrix_.m[0][2]  // x軸のz成分
	};
	right_ = Normalize(right_);

	move_ = { 0.0f, 0.0f, 0.0f };

	if (key[DIK_W]) {
		move_ += forward_ * speed_;
	}
	if (key[DIK_S]) {
		move_ -= forward_ * speed_;
	}
	if (key[DIK_D]) {
		move_ += right_ * speed_;
	}
	if (key[DIK_A]) {
		move_ -= right_ * speed_;
	}

	transform_.translate += move_;

	if (key[DIK_SPACE]) {
		transform_.translate.y += speed_;
	}
	if (key[DIK_LSHIFT]) {
		transform_.translate.y -= speed_;
	}

	//マウスで視点移動
	//回転処理(左クリックしながらドラッグ)
	// カーソル非表示
	if (mouseInput->IsLeftTriggered()) {
		ShowCursor(FALSE);
	}
	if (mouseInput->IsLeftReleased()) {
		ShowCursor(TRUE);
	}

	if (mouseInput->left) {
		// 画面中央に戻す
		POINT center;
		center.x = 1280 / 2;
		center.y = 720 / 2;
		ClientToScreen(hwnd, &center); // hwndは自分のウィンドウハンドル
		SetCursorPos(center.x, center.y);

		transform_.rotate.y += mouseInput->x * sensitivity_;
		transform_.rotate.x += mouseInput->y * sensitivity_;

		if (transform_.rotate.x > pitchOver_) {
			transform_.rotate.x = pitchOver_;
		}
		if (transform_.rotate.x < -pitchOver_) {
			transform_.rotate.x = -pitchOver_;
		}
	}

	//変化した情報をworldMatrixにまとめてviewMatrixに入れる
	worldMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	viewMatrix_ = Inverse(worldMatrix_);
}