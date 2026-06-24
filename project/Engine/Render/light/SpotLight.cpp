#include "PCH.h"
#include "SpotLight.h"
#include "imgui.h"
#include "MathFunction.h"

// フロート方の最大値を定義
static inline const float kMax_Float = 99999.9f;

SpotLight::SpotLight() {
	color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
	intensity_ = 1.0f;
	position_ = { -3.0f, 0.0f, 0.0f };
	direction_ = { 1.0f, 0.0f, 0.0f };
	distance_ = 10.0f;
	decay_ = 1.0f;
	cosAngle_ = 0.0f;
}

SpotLight::~SpotLight() {

}

void SpotLight::Initialize() {
	color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
	intensity_ = 1.0f;
	position_ = { -3.0f, 0.0f, 0.0f };
	direction_ = { 1.0f, 0.0f, 0.0f };
	distance_ = 10.0f;
	decay_ = 1.0f;
	cosAngle_ = 0.0f;
}

void SpotLight::Update() {
	direction_ = Math::Normalize(direction_);
}

void SpotLight::ImGui(int index) {
#ifdef USEIMGUI
	// indexを使ってラベルをユニークにする
	std::string label = "SpontLightSettings ##" + std::to_string(index);
	if(ImGui::CollapsingHeader(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::ColorEdit4("Color", &color_.x);
		ImGui::DragFloat3("Position", &position_.x, 0.01f);
		ImGui::DragFloat("Intensity", &intensity_, 0.01f, 0.0f, kMax_Float);
		ImGui::DragFloat3("Direction", &direction_.x, 0.01f, -1.0f, 1.0f);
		ImGui::DragFloat("Distance", &distance_, 0.01f, 0.0f, kMax_Float);
		ImGui::DragFloat("Decay", &decay_, 0.01f, 0.0f, kMax_Float);
		ImGui::DragFloat("CosAngle", &cosAngle_, 0.01f, -1.0f, 1.0f);
	}
#endif
}