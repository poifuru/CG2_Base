#include "DirectionalLight.h"
#include <string>
#include "imgui.h"

DirectionalLight::DirectionalLight() {
	color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
	intensity_ = 1.0f;
	direction_ = {0.0f, -1.0f, 0.0f};

	myIndex_ = numInstance_;
	numInstance_++;
}

DirectionalLight::~DirectionalLight() {

}

void DirectionalLight::Initialize() {
	color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
	intensity_ = 1.0f;
	direction_ = { 0.0f, -1.0f, 0.0f };
}

void DirectionalLight::Update() {
#ifdef USEIMGUI
	// 最初のインスタンス（myIndex_ == 0）の時だけ、選択用のスライダーを出す
	if(myIndex_ == 0) {
		ImGui::Begin("Light Manager");
		ImGui::SliderInt("Select Light", &selectIndex_, 0, numInstance_ - 1);
		ImGui::End();
	}

	// 選択されている番号が自分の番号と一致する場合のみ、編集UIを出す
	if(selectIndex_ == (int)myIndex_) {
		std::string label = "DirectionalLight Settings ##" + std::to_string(myIndex_);
		if(ImGui::CollapsingHeader(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::ColorEdit4("Color", &color_.x);
			ImGui::DragFloat3("Direction", &direction_.x, 0.01f, -1.0f, 1.0f);
			ImGui::DragFloat("Intensity", &intensity_, 0.01f, 0.0f, 10.0f);
		}
	}
#endif
}
