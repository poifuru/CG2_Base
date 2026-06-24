#include "PCH.h"
#include "LookAtCamera.h"
#include "MathFunction.h"
#include "WindowsAPI.h"
#include "InputManager.h"
#include "RawInput.h"

LookAtCamera::LookAtCamera () {
	input_ = InputManager::GetInstance();
	sensitivity_ = 0.0f;
	pitchOver_ = 0.0f;
}

LookAtCamera::~LookAtCamera () {

}

void LookAtCamera::Initialize (const EulerTransform& transform) {
	camera_.transform = transform;

	//LookAtCameraの初期値
	target_ = {};
	distance_ = 15.0f;

	camera_.transform.rotate.x = 0.3f;
	//camera_.transform.rotate.y = -1.0f;

	sensitivity_ = 0.001f;
	pitchOver_ = 1.5708f;

	camera_.proj = Math::MakePerspectiveFOVMatrix (0.45f, WindowsAPI::GetInstance()->GetAspectRatio(), 0.1f, 1000.0f);
}

void LookAtCamera::Update () {
	//マウスで視点移動
	//回転処理(左クリックしながらドラッグ)
	// カーソル非表示
	if (input_->GetRawInput ()->PushMouse (MouseButton::MIDDLE)) {
		ShowCursor (FALSE);
		//ターゲットの球面座標上を回転させる
		camera_.transform.rotate.y += input_->GetRawInput ()->GetMouseDeltaX () * sensitivity_;
		camera_.transform.rotate.x += input_->GetRawInput ()->GetMouseDeltaY () * sensitivity_;

		//視点移動の制限
		if (camera_.transform.rotate.x > pitchOver_) {
			camera_.transform.rotate.x = pitchOver_;
		}
		if (camera_.transform.rotate.x < -pitchOver_) {
			camera_.transform.rotate.x = -pitchOver_;
		}
	}
	else {
		// カーソルの制限を解除（NULLを指定）
		ClipCursor (NULL);
		ShowCursor (TRUE);
	}

	// === 🌟 LookAtカメラの視点位置計算 🌟 ===
	// Y軸回転（水平方向）を yaw_、X軸回転（垂直方向）を pitch_ として扱う
	float yaw_ = camera_.transform.rotate.y;
	float pitch_ = camera_.transform.rotate.x;

	Vector3 eyePosition = {};
	// 極座標/球面座標で視点を計算
	// Y成分（高さ）
	eyePosition.y = target_.y + distance_ * std::sin (pitch_);

	// 水平方向の半径
	float horizontalDistance = distance_ * std::cos (pitch_);

	// X成分とZ成分
	eyePosition.x = target_.x + horizontalDistance * std::sin (yaw_);
	eyePosition.z = target_.z - horizontalDistance * std::cos (yaw_);

	//変化した情報をカメラに代入
	camera_.view = Math::MakeLookAtMatrix (eyePosition, target_, upVector_);
	camera_.vp = Math::Multiply (camera_.view, camera_.proj);
}

void LookAtCamera::ImGui () {
	std::string ID = std::to_string (instanceNum_);
	std::string label = "LookAtCamera";

	ImGui::DragFloat3 (("scale##" + label + ID).c_str (), &camera_.transform.scale.x, 0.01f);
	ImGui::DragFloat3 (("rotate##" + label + ID).c_str (), &camera_.transform.rotate.x, 0.01f);
	ImGui::DragFloat3 (("translate##" + label + ID).c_str (), &camera_.transform.translate.x, 0.01f);
}
