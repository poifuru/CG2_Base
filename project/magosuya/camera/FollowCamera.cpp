#include "FollowCamera.h"
#include <imgui.h>
#include "WindowsAPI.h"

FollowCamera::FollowCamera () {
	camera_.transform = {};
	camera_.world = Math::MakeIdentity4x4 ();
	camera_.view = Math::MakeIdentity4x4 ();
	camera_.proj = Math::MakeIdentity4x4 ();

	offset_ = { 0.0f, 0.0f, -80.0f };

	instanceNum_++;
}

FollowCamera::~FollowCamera () {
	
}

void FollowCamera::Initialize (const EulerTransform& transform) {
	camera_.transform = transform;
	camera_.proj = Math::MakePerspectiveFOVMatrix (0.45f, (float)WindowsAPI::GetInstance ()->kClientWidth / (float)WindowsAPI::GetInstance ()->kClientHeight, 0.1f, 1000.0f);
}

void FollowCamera::SetTarget (const EulerTransform* target) {
	target_ = const_cast<EulerTransform*>(target);
}

void FollowCamera::Update () {
	//追従対象がいるかチェック
	if (!target_) {
		//居なければ早期リターン
		return;
	}

	camera_.transform.translate.z = target_->translate.z + offset_.z;

	//行列の計算
	camera_.world = Math::MakeAffineMatrix (camera_.transform.scale, camera_.transform.rotate, camera_.transform.translate);
	camera_.view = Math::Inverse (camera_.world);
	camera_.vp = Math::Multiply (camera_.view, camera_.proj);
}

void FollowCamera::ImGui () {
	std::string ID = std::to_string (instanceNum_);
	std::string label = "FollowCamera" + ID;

	ImGui::DragFloat3 (("scale##" + label + ID).c_str (), &camera_.transform.scale.x, 0.01f);
	ImGui::DragFloat3 (("rotate##" + label + ID).c_str (), &camera_.transform.rotate.x, 0.01f);
	ImGui::DragFloat3 (("translate##" + label + ID).c_str (), &camera_.transform.translate.x, 0.01f);
}