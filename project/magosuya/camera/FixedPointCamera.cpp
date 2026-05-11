#include "FixedPointCamera.h"
#include <imgui.h>
#include "MathFunction.h"
#include "WindowsAPI.h"

FixedPointCamera::FixedPointCamera () {
	camera_.Transform = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -30.0f} };
	camera_.world = Math::MakeIdentity4x4 ();
	camera_.view = Math::MakeIdentity4x4 ();
	camera_.proj = Math::MakeIdentity4x4 ();

	instanceNum_++;
}

FixedPointCamera::~FixedPointCamera () {

}

void FixedPointCamera::Initialize (const EulerTransform& Transform) {
	camera_.Transform = Transform;
	camera_.proj = Math::MakePerspectiveFOVMatrix (0.45f, (float)WindowsAPI::GetInstance ()->kClientWidth / (float)WindowsAPI::GetInstance ()->kClientHeight, 0.1f, 1000.0f);
}

void FixedPointCamera::Update () {
	camera_.world = Math::MakeAffineMatrix (camera_.Transform.scale, camera_.Transform.rotate, camera_.Transform.translate);
	camera_.view = Math::Inverse (camera_.world);
	camera_.vp = Math::Multiply (camera_.view, camera_.proj);
}

void FixedPointCamera::ImGui () {
	std::string ID = std::to_string (instanceNum_);
	std::string label = "FixedPointCamera";

	ImGui::DragFloat3 (("scale##" + label + ID).c_str (), &camera_.Transform.scale.x, 0.01f);
	ImGui::DragFloat3 (("rotate##" + label + ID).c_str (), &camera_.Transform.rotate.x, 0.01f);
	ImGui::DragFloat3 (("translate##" + label + ID).c_str (), &camera_.Transform.translate.x, 0.01f);
}