#include "DebugCamera.h"
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma	comment(lib, "dxguid.lib")
#include <string>
#include <imgui.h>
#include "WindowsAPI.h"

DebugCamera::DebugCamera() {
	transform_ = {};
	world_ = {};
	view_ = {};
	proj_ = {};

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
	transform_ = transform;
	world_ = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	view_ = Inverse(world_);
	proj_ = MakePerspectiveFOVMatrix(0.45f, winAPI_->kClientWidth / winAPI_->kClientHeight, 0.1f, 100.0f);

	speed_ = 0.1f;
}

void DebugCamera::Update() {
	//=======更新処理=======//
	//カメラの前後左右の移動
	forward_ = {
	world_.m[2][0], // z軸のx成分
	world_.m[2][1], // z軸のy成分
	world_.m[2][2]  // z軸のz成分
	};
	forward_ = Normalize(forward_);

	right_ = {
	world_.m[0][0], // x軸のx成分
	world_.m[0][1], // x軸のy成分
	world_.m[0][2]  // x軸のz成分
	};
	right_ = Normalize(right_);

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

	transform_.translate += move_;

	if (inputManager_->GetRawInput ()->Push (VK_SPACE)) {
		transform_.translate.y += speed_;
	}
	if (inputManager_->GetRawInput ()->Push (VK_SHIFT)) {
		transform_.translate.y -= speed_;
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
		transform_.rotate.y += inputManager_->GetRawInput ()->GetMouseDeltaX() * sensitivity_;
		transform_.rotate.x += inputManager_->GetRawInput ()->GetMouseDeltaY () * sensitivity_;

		if (transform_.rotate.x > pitchOver_) {
			transform_.rotate.x = pitchOver_;
		}
		if (transform_.rotate.x < -pitchOver_) {
			transform_.rotate.x = -pitchOver_;
		}
	}

	//変化した情報をworldMatrixにまとめてviewMatrixに入れる
	world_ = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	view_ = Inverse(world_);
}

void DebugCamera::ImGui () {
	std::string ID = std::to_string (instanceNum_);
	std::string label = "DebugCamera";

	ImGui::Text ((label + ID).c_str ());
	ImGui::DragFloat3 (("scale##" + label + ID).c_str (), &transform_.scale.x, 0.01f);
	ImGui::DragFloat3 (("rotate##" + label + ID).c_str (), &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3 (("translate##" + label + ID).c_str (), &transform_.translate.x, 0.01f);
}