#include "FollowCamera.h"
#include <imgui.h>

FollowCamera::FollowCamera () {
	transform_ = {};
	world_ = MakeIdentity4x4 ();
	view_ = MakeIdentity4x4 ();
	proj_ = MakeIdentity4x4 ();

	target_ = new Vector3;

	winAPI_ = WindowsAPI::GetInstance ();

	instanceNum_++;
}

FollowCamera::~FollowCamera () {
	delete target_;
}

void FollowCamera::Initialize (const Transform& transform) {
	transform_ = transform;

	//オフセットの設定
}

void FollowCamera::SetTarget (const Vector3* target) {

}

void FollowCamera::Update () {

}

void FollowCamera::ImGui () {
	std::string ID = std::to_string (instanceNum_);
	std::string label = "FollowCamera";

	ImGui::Text ((label + ID).c_str ());
	ImGui::DragFloat3 (("scale##" + label + ID).c_str (), &transform_.scale.x, 0.01f);
	ImGui::DragFloat3 (("rotate##" + label + ID).c_str (), &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3 (("translate##" + label + ID).c_str (), &transform_.translate.x, 0.01f);
}