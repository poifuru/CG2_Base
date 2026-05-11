#include "FollowCamera.h"
#include <imgui.h>
#include "WindowsAPI.h"

FollowCamera::FollowCamera () {
	camera_.Transform = {};
	camera_.world = Math::MakeIdentity4x4 ();
	camera_.view = Math::MakeIdentity4x4 ();
	camera_.proj = Math::MakeIdentity4x4 ();

	offset_ = { 0.0f, 0.0f, -50.0f };

	instanceNum_++;
}

FollowCamera::~FollowCamera () {
	
}

void FollowCamera::Initialize (const EulerTransform& Transform) {
	camera_.Transform = Transform;
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

	camera_.Transform.translate = target_->translate + offset_;

	if(camera_.Transform.translate.x <= 19.0f) {
		camera_.Transform.translate.x = 19.0f;
	}
	else if(camera_.Transform.translate.x >= 85.0f) {
		camera_.Transform.translate.x = 85.0f;
	}

	//行列の計算
	camera_.world = Math::MakeAffineMatrix (camera_.Transform.scale, camera_.Transform.rotate, camera_.Transform.translate);
	camera_.view = Math::Inverse (camera_.world);
	camera_.vp = Math::Multiply (camera_.view, camera_.proj);
}

void FollowCamera::ImGui () {
	std::string ID = std::to_string (instanceNum_);
	std::string label = "FollowCamera" + ID;

	ImGui::DragFloat3 (("scale##" + label + ID).c_str (), &camera_.Transform.scale.x, 0.01f);
	ImGui::DragFloat3 (("rotate##" + label + ID).c_str (), &camera_.Transform.rotate.x, 0.01f);
	ImGui::DragFloat3 (("translate##" + label + ID).c_str (), &camera_.Transform.translate.x, 0.01f);
}