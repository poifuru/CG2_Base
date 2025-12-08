#include "FollowCamera.h"
#include <imgui.h>
#include "WindowsAPI.h"

FollowCamera::FollowCamera () {
	camera_.transform = {};
	camera_.world = Math::MakeIdentity4x4 ();
	camera_.view = Math::MakeIdentity4x4 ();
	camera_.proj = Math::MakeIdentity4x4 ();

	offset_ = { 0.0f, 4.0f, -15.0f };
	smoothness_ = 0.1f;

	instanceNum_++;
}

FollowCamera::~FollowCamera () {
	
}

void FollowCamera::Initialize (const Transform& transform) {
	camera_.transform = transform;
	camera_.proj = Math::MakePerspectiveFOVMatrix (0.45f, (float)WindowsAPI::GetInstance ()->kClientWidth / (float)WindowsAPI::GetInstance ()->kClientHeight, 0.1f, 1000.0f);
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
	Vector3 targetPos = target_->translate + offset_;
	camera_.transform.translate = Math::Lerp (camera_.transform.translate, targetPos, smoothness_);

	//カメラの回転もターゲットに合わせる
	//camera_.transform.rotate = Math::Lerp (camera_.transform.rotate, target_->rotate, smoothness_);

	//行列の計算
	camera_.world = Math::MakeAffineMatrix (camera_.transform.scale, camera_.transform.rotate, camera_.transform.translate);
	camera_.view = Math::Inverse (camera_.world);
	camera_.vp = Math::Multiply (camera_.view, camera_.proj);
}

void FollowCamera::ImGui () {
	std::string ID = std::to_string (instanceNum_);
	std::string label = "FollowCamera" + ID;

	ImGui::DragFloat (("smoothness##" + label).c_str(), &smoothness_, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat3 (("scale##" + label + ID).c_str (), &camera_.transform.scale.x, 0.01f);
	ImGui::DragFloat3 (("rotate##" + label + ID).c_str (), &camera_.transform.rotate.x, 0.01f);
	ImGui::DragFloat3 (("translate##" + label + ID).c_str (), &camera_.transform.translate.x, 0.01f);
}