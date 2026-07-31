#include "PCH.h"
#include "DirectionalLight.h"
#include "MathFunction.h"

// フロート型の最大値を定義
static inline const float kMax_Float = 99999.9f;

DirectionalLight::DirectionalLight() {
	color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
	intensity_ = 1.0f;
	direction_ = {0.0f, 0.0f, 1.0f};
}

DirectionalLight::~DirectionalLight() {

}

void DirectionalLight::Initialize() {
	color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
	intensity_ = 1.0f;
	direction_ = { 0.0f, -1.0f, 0.0f };
}

void DirectionalLight::Update() {
	direction_ = Math::Normalize(direction_);
}

void DirectionalLight::ImGui(int index) {
#ifdef USEIMGUI
	// indexを使ってラベルをユニークにする
	std::string label = "DirectionalLightSettings ##" + std::to_string(index);
	if(ImGui::CollapsingHeader(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::ColorEdit4("Color", &color_.x);
		ImGui::DragFloat3("Direction", &direction_.x, 0.01f, -1.0f, 1.0f);
		ImGui::DragFloat("Intensity", &intensity_, 0.01f, 0.0f, kMax_Float);
	}
#endif
}