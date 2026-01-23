#include "RectLight.h"
#include "imgui.h"
#include "MathFunction.h"

RectLight::RectLight() {
	color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
	intensity_ = 1.0f;
	position_ = { 0.0f, 5.0f, 0.0f };
	direction_ = { 0.0f, -1.0f, 0.0f };
	size_ = { 1.0f, 1.0f };
	right_ = { 1.0f, 0.0f, 0.0f };
	up_ = { 0.0f, 0.0f, 1.0f };
	decay_ = 1.0f;
}

RectLight::~RectLight() {

}

void RectLight::Initialize() {
	color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
	intensity_ = 1.0f;
	position_ = { -3.0f, 0.0f, 0.0f };
	direction_ = { 0.0f, -1.0f, 0.0f };
	size_ = { 10.0f, 10.0f };
	right_ = { 1.0f, 1.0f, 1.0f };
	up_ = { 1.0f, 1.0f, 1.0f };
	decay_ = 1.0f;
}

void RectLight::Update() {
	direction_ = Math::Normalize(direction_);
	right_ = Math::Normalize(right_);
	up_ = Math::Normalize(up_);
}

void RectLight::ImGui(int index) {
#ifdef USEIMGUI
	// indexを使ってラベルをユニークにする
	std::string label = "SpontLightSettings ##" + std::to_string(index);
	if(ImGui::CollapsingHeader(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::ColorEdit4("Color", &color_.x);
		ImGui::DragFloat3("Position", &position_.x, 0.01f, -100.0f, 100.0f);
		ImGui::DragFloat("Intensity", &intensity_, 0.01f, 0.0f, 10.0f);
		ImGui::DragFloat3("Direction", &direction_.x, 0.01f, -1.0f, 1.0f);
		ImGui::DragFloat2("Size", &size_.x, 0.01f, 0.0f, 10.0f);
		ImGui::DragFloat3("RightVector", &right_.x, 0.01f, -1.0f, 1.0f);
		ImGui::DragFloat3("UpVector", &up_.x, 0.01f, -1.0f, 1.0f);
	}
#endif
}