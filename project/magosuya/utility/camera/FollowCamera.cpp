#include "FollowCamera.h"
#include <imgui.h>

FollowCamera::FollowCamera () {
	camera_->transform = {};
	camera_->world = Math::MakeIdentity4x4 ();
	camera_->view = Math::MakeIdentity4x4 ();
	camera_->proj = Math::MakeIdentity4x4 ();

	offset_ = { 0.0f, 2.0f, -5.0f };
	smoothness_ = 0.1f;

	winAPI_ = WindowsAPI::GetInstance ();

	instanceNum_++;
}

FollowCamera::~FollowCamera () {
	
}

void FollowCamera::Initialize (const Transform& transform) {
	camera_->transform = transform;
	camera_->proj = Math::MakePerspectiveFOVMatrix (0.45f, winAPI_->kClientWidth / winAPI_->kClientHeight, 0.1f, 1000.0f);
}

void FollowCamera::SetTarget (const Transform* target) {
	target_ = const_cast<Transform*>(target);
}

void FollowCamera::Update () {
	//追従対象がいるかチェック
	if (!target_) {
		//居なければ早期リターン
		return;
	}

	//ターゲットの位置とオフセットからカメラの位置を計算
	// ターゲットの回転行列を作成
	Matrix4x4 rotateMat = Math::MakeRotateXMatrix (target_->rotate.x);
	rotateMat = Math::Multiply(rotateMat, (Math::MakeRotateYMatrix (target_->rotate.y)));
	rotateMat = Math::Multiply(rotateMat, (Math::MakeRotateZMatrix (target_->rotate.z)));

	// オフセットをターゲットの回転で回す
	Vector3 rotatedOffset = Math::Transform(offset_, rotateMat);

	// 理想のカメラ位置は、ターゲットの位置 + 回転したオフセット
	Vector3 cameraPos = target_->translate + rotatedOffset;
	camera_->transform.translate = Math::Lerp (camera_->transform.translate, cameraPos, smoothness_);

	//行列の計算
	camera_->world = Math::MakeAffineMatrix (camera_->transform.scale, camera_->transform.rotate, camera_->transform.translate);
	camera_->view = Math::Inverse (camera_->world);

}

void FollowCamera::ImGui () {
	std::string ID = std::to_string (instanceNum_);
	std::string label = "FollowCamera" + ID;

	ImGui::Text (label.c_str ());
	ImGui::DragFloat (("smoothness##" + label).c_str(), &smoothness_, 0.01f, 0.0f, 1.0f);
}