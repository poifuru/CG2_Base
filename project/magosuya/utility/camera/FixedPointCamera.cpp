#include "FixedPointCamera.h"
#include <imgui.h>
#include "MathFunction.h"
#include "WindowsAPI.h"

FixedPointCamera::FixedPointCamera () {
	transform_ = {};
	world_ = MakeIdentity4x4 ();
	view_ = MakeIdentity4x4 ();
	proj_ = MakeIdentity4x4 ();

	inputManager_ = InputManager::GetInstance ();
	winAPI_ = WindowsAPI::GetInstance ();

	instanceNum_++;
}

FixedPointCamera::~FixedPointCamera () {

}

void FixedPointCamera::Initialize (const Transform& transform) {
	transform_ = transform;
}

void FixedPointCamera::Update () {
	world_ = MakeAffineMatrix (transform_.scale, transform_.rotate, transform_.translate);
	view_ = Inverse (world_);
	proj_ = MakePerspectiveFOVMatrix (0.45f, winAPI_->kClientWidth / winAPI_->kClientHeight, 0.1f, 1000.0f);
}

void FixedPointCamera::ImGui () {
	std::string ID = std::to_string (instanceNum_);
	std::string label = "FixedPointCamera";

	ImGui::Text ((label + ID).c_str ());
	ImGui::DragFloat3 (("scale##" + label + ID).c_str (), &transform_.scale.x, 0.01f);
	ImGui::DragFloat3 (("rotate##" + label + ID).c_str (), &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3 (("translate##" + label + ID).c_str (), &transform_.translate.x, 0.01f);
}