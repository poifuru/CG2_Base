#include "PointLight.h"
#include "imgui.h"

PointLight::PointLight() {
	color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
	intensity_ = 1.0f;
	position_ = { 0.0f, 0.0f, -5.0f };
	radius_ = 10.0f;
	decay_ = 1.0f;
}

PointLight::~PointLight() {

}

void PointLight::Initialize() {
	color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
	intensity_ = 1.0f;
	position_ = { 0.0f, 0.0f, -5.0f };
	radius_ = 10.0f;
	decay_ = 1.0f;
}

void PointLight::Update() {

}

void PointLight::ImGui(int index) {
#ifdef USEIMGUI
	// indexを使ってラベルをユニークにする
	std::string label = "PointLightSettings ##" + std::to_string(index);
	if(ImGui::CollapsingHeader(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::ColorEdit4("Color", &color_.x);
		ImGui::DragFloat3("Position", &position_.x, 0.01f);
		ImGui::DragFloat("Intensity", &intensity_, 0.01f, 0.0f, IMGUI_MAX);
		ImGui::DragFloat("radius", &radius_, 0.01f, 0.0f, IMGUI_MAX);
		ImGui::DragFloat("decay", &decay_, 0.01f, 0.0f, IMGUI_MAX);
	}
#endif
}