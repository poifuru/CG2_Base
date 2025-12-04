#include "DebugCamera.h"
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma	comment(lib, "dxguid.lib")
#include <string>
#include <imgui.h>
#include "WindowsAPI.h"

DebugCamera::DebugCamera() {
	camera_.transform = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -30.0f} };
	camera_.world = {};
	camera_.view = {};
	camera_.proj = {};

	forward_ = {};
	right_ = {};
	speed_ = 0.0f;
	move_ = {};

	sensitivity_ = 0.001f;
	pitchOver_ = 1.5708f;

	inputManager_ = InputManager::GetInstance ();
	winAPI_ = WindowsAPI::GetInstance ();

	instanceNum_++;
}

DebugCamera::~DebugCamera () {

}

void DebugCamera::Initialize(const Transform& transform) {
	camera_.transform = transform;
	camera_.world = Math::MakeAffineMatrix (camera_.transform.scale, camera_.transform.rotate, camera_.transform.translate);
	camera_.view = Math::Inverse (camera_.world);
	camera_.proj = Math::MakePerspectiveFOVMatrix(0.45f, (float)winAPI_->kClientWidth / (float)winAPI_->kClientHeight, 0.1f, 1000.0f);

	speed_ = 0.1f;
}

void DebugCamera::Update() {
	//=======更新処理=======//
	//カメラの前後左右の移動
	forward_ = {
	camera_.world.m[2][0], // z軸のx成分
	camera_.world.m[2][1], // z軸のy成分
	camera_.world.m[2][2]  // z軸のz成分
	};
	forward_ = Math::Normalize(forward_);

	right_ = {
	camera_.world.m[0][0], // x軸のx成分
	camera_.world.m[0][1], // x軸のy成分
	camera_.world.m[0][2]  // x軸のz成分
	};
	right_ = Math::Normalize(right_);

	move_ = { 0.0f, 0.0f, 0.0f };

	if (inputManager_->GetRawInput()->Push('W')) {
		move_ += forward_ * speed_;
	}
	if (inputManager_->GetRawInput ()->Push ('S')) {
		move_ -= forward_ * speed_;
	}
	if (inputManager_->GetRawInput ()->Push ('D')) {
		move_ += right_ * speed_;
	}
	if (inputManager_->GetRawInput ()->Push ('A')) {
		move_ -= right_ * speed_;
	}

	camera_.transform.translate += move_;

	if (inputManager_->GetRawInput ()->Push (VK_SPACE)) {
		camera_.transform.translate.y += speed_;
	}
	if (inputManager_->GetRawInput ()->Push (VK_SHIFT)) {
		camera_.transform.translate.y -= speed_;
	}

	//マウスで視点移動
	//回転処理(左クリックしながらドラッグ)
	// カーソル非表示
	if (inputManager_->GetRawInput ()->TriggerMouse(MouseButton::MIDDLE)) {
		ShowCursor(FALSE);

		// クライアント領域の矩形を取得
		RECT clientRect;
		GetClientRect (winAPI_->GetHwnd(), &clientRect);

		// クライアント領域の座標をスクリーン座標に変換するでやんす
		// ClipCursorはスクリーン座標を要求するからでやんす
		POINT pt = { clientRect.left, clientRect.top };
		ClientToScreen (winAPI_->GetHwnd (), &pt);
		clientRect.left = pt.x;
		clientRect.top = pt.y;

		pt.x = clientRect.right;
		pt.y = clientRect.bottom;
		ClientToScreen (winAPI_->GetHwnd (), &pt);
		clientRect.right = pt.x;
		clientRect.bottom = pt.y;

		// カーソルをウィンドウのクライアント領域に制限するでやんす！
		ClipCursor (&clientRect);
	}
	if (inputManager_->GetRawInput ()->ReleaseMouse (MouseButton::MIDDLE)) {
		// カーソルの制限を解除（NULLを指定）
		ClipCursor (NULL);
		ShowCursor (TRUE);
	}

	if (inputManager_->GetRawInput ()->PushMouse (MouseButton::MIDDLE)) {
		camera_.transform.rotate.y += inputManager_->GetRawInput ()->GetMouseDeltaX() * sensitivity_;
		camera_.transform.rotate.x += inputManager_->GetRawInput ()->GetMouseDeltaY () * sensitivity_;

		if (camera_.transform.rotate.x > pitchOver_) {
			camera_.transform.rotate.x = pitchOver_;
		}
		if (camera_.transform.rotate.x < -pitchOver_) {
			camera_.transform.rotate.x = -pitchOver_;
		}
	}

	//変化した情報をworldMatrixにまとめてviewMatrixに入れる
	camera_.world = Math::MakeAffineMatrix(camera_.transform.scale, camera_.transform.rotate, camera_.transform.translate);
	camera_.view = Math::Inverse(camera_.world);
	camera_.vp = Math::Multiply (camera_.view, camera_.proj);
}

void DebugCamera::ImGui () {
	std::string ID = std::to_string (instanceNum_);
	std::string label = "DebugCamera";

	ImGui::DragFloat3 (("scale##" + label + ID).c_str (), &camera_.transform.scale.x, 0.01f);
	ImGui::DragFloat3 (("rotate##" + label + ID).c_str (), &camera_.transform.rotate.x, 0.01f);
	ImGui::DragFloat3 (("translate##" + label + ID).c_str (), &camera_.transform.translate.x, 0.01f);
}