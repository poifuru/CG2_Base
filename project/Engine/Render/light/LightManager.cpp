#include "PCH.h"
#include "LightManager.h"
#include <algorithm>
#include <cassert>
#ifdef USEIMGUI
#include "imgui.h"
#endif

void LightManager::Initialize(ID3D12Device* device) {
	assert(device != nullptr);
	lightBuffer_.Initialize(device);
}

void LightManager::Update() {
	//各ライトを更新
	for(auto& light : dirLights_) {	//directionalLight
		light->Update();
	}

	for(auto& light : pointLights_) {	//pointLight
		light->Update();
	}

	for(auto& light : spotLights_) {
		light->Update();
	}

	for(auto& light : rectLights_) {
		light->Update();
	}

	//LightCountのデータを更新
	lightCPUData_.count.pointLight = static_cast<int>(pointLights_.size());
	lightCPUData_.count.dirLight = static_cast<int>(dirLights_.size());
	lightCPUData_.count.spotLight = static_cast<int>(spotLights_.size());
	lightCPUData_.count.rectLight = static_cast<int>(rectLights_.size());

	// DirectionalLightデータを詰める
	size_t count = (std::min)(dirLights_.size(), (size_t)MaxCount);
	for(size_t i = 0; i < count; ++i) {
		lightCPUData_.dirLights[i].color = dirLights_[i]->GetColor();
		lightCPUData_.dirLights[i].direction = dirLights_[i]->GetDirection();
		lightCPUData_.dirLights[i].intensity = dirLights_[i]->GetIntensity();
	}

	// PointLightデータを詰める
	count = (std::min)(pointLights_.size(), (size_t)MaxCount);
	for(size_t i = 0; i < count; ++i) {
		lightCPUData_.pointLights[i].color = pointLights_[i]->GetColor();
		lightCPUData_.pointLights[i].position = pointLights_[i]->GetPosition();
		lightCPUData_.pointLights[i].intensity = pointLights_[i]->GetIntensity();
		lightCPUData_.pointLights[i].radius = pointLights_[i]->GetRadius();
		lightCPUData_.pointLights[i].decay = pointLights_[i]->GetDecay();
	}

	// SpotLightデータを詰める
	count = (std::min)(spotLights_.size(), (size_t)MaxCount);
	for(size_t i = 0; i < count; ++i) {
		lightCPUData_.spotLights[i].color = spotLights_[i]->GetColor();
		lightCPUData_.spotLights[i].position = spotLights_[i]->GetPosition();
		lightCPUData_.spotLights[i].intensity = spotLights_[i]->GetIntensity();
		lightCPUData_.spotLights[i].direction = spotLights_[i]->GetDirection();
		lightCPUData_.spotLights[i].distance = spotLights_[i]->GetDistance();
		lightCPUData_.spotLights[i].decay = spotLights_[i]->GetDecay();
		lightCPUData_.spotLights[i].cosAngle = spotLights_[i]->GetCosAngle();
	}

	// RectLightデータを詰める
	count = (std::min)(rectLights_.size(), (size_t)MaxCount);
	for(size_t i = 0; i < count; ++i) {
		lightCPUData_.rectLights[i].color = rectLights_[i]->GetColor();
		lightCPUData_.rectLights[i].position = rectLights_[i]->GetPosition();
		lightCPUData_.rectLights[i].intensity = rectLights_[i]->GetIntensity();
		lightCPUData_.rectLights[i].direction = rectLights_[i]->GetDirection();
		lightCPUData_.rectLights[i].size = rectLights_[i]->GetSize();
		lightCPUData_.rectLights[i].right = rectLights_[i]->GetRight();
		lightCPUData_.rectLights[i].up = rectLights_[i]->GetUp();
		lightCPUData_.rectLights[i].decay = rectLights_[i]->GetDecay();
	}

	// 定数バッファ全体をGPUへ転送
	lightBuffer_.Update(lightCPUData_);
}

void LightManager::ImGui() {
#ifdef USEIMGUI
	if(ImGui::BeginTabBar("LightTabs")) {
		if(ImGui::BeginTabItem("Directional")) {
			if(ImGui::Button("Add DirectionalLight")) {
				if(dirLights_.size() < MaxCount) {
					AddLight(DIRECTIONALLIGHT);
					selectDirLightIndex_ = static_cast<int>(dirLights_.size()) - 1;
				}
			}
			if(dirLights_.empty()) {
				ImGui::Text("No Directional Lights");
			}
			else {
				int maxIdx = static_cast<int>(dirLights_.size()) - 1;
				ImGui::SliderInt("Select", &selectDirLightIndex_, 0, maxIdx);
				ImGui::Separator();
				if(selectDirLightIndex_ >= 0 && selectDirLightIndex_ < (int)dirLights_.size()) {
					dirLights_[selectDirLightIndex_]->ImGui(selectDirLightIndex_);
					ImGui::Spacing();
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
					if(ImGui::Button("Delete This Light", ImVec2(-1, 0))) {
						dirLights_.erase(dirLights_.begin() + selectDirLightIndex_);
						if(selectDirLightIndex_ > 0) {
							selectDirLightIndex_--;
						}
					}
					ImGui::PopStyleColor(2);
				}
			}
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("Point")) {
			if(ImGui::Button("Add PointLight")) {
				if(pointLights_.size() < MaxCount) {
					AddLight(POINTLIGHT);
					selectPointLightIndex_ = static_cast<int>(pointLights_.size()) - 1;
				}
			}
			if(pointLights_.empty()) {
				ImGui::Text("No Point Lights");
			}
			else {
				int maxIdx = static_cast<int>(pointLights_.size()) - 1;
				ImGui::SliderInt("Select", &selectPointLightIndex_, 0, maxIdx);
				ImGui::Separator();
				if(selectPointLightIndex_ >= 0 && selectPointLightIndex_ < (int)pointLights_.size()) {
					pointLights_[selectPointLightIndex_]->ImGui(selectPointLightIndex_);
					ImGui::Spacing();
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
					if(ImGui::Button("Delete This Light", ImVec2(-1, 0))) {
						pointLights_.erase(pointLights_.begin() + selectPointLightIndex_);
						if(selectPointLightIndex_ > 0) {
							selectPointLightIndex_--;
						}
					}
					ImGui::PopStyleColor(2);
				}
			}
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("Spot")) {
			if(ImGui::Button("Add SpotLight")) {
				if(spotLights_.size() < MaxCount) {
					AddLight(SPOTLIGHT);
					selectSpotLightIndex_ = static_cast<int>(spotLights_.size()) - 1;
				}
			}
			if(spotLights_.empty()) {
				ImGui::Text("No Spot Lights");
			}
			else {
				int maxIdx = static_cast<int>(spotLights_.size()) - 1;
				ImGui::SliderInt("Select", &selectSpotLightIndex_, 0, maxIdx);
				ImGui::Separator();
				if(selectSpotLightIndex_ >= 0 && selectSpotLightIndex_ < (int)spotLights_.size()) {
					spotLights_[selectSpotLightIndex_]->ImGui(selectSpotLightIndex_);
					ImGui::Spacing();
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
					if(ImGui::Button("Delete This Light", ImVec2(-1, 0))) {
						spotLights_.erase(spotLights_.begin() + selectSpotLightIndex_);
						if(selectSpotLightIndex_ > 0) {
							selectSpotLightIndex_--;
						}
					}
					ImGui::PopStyleColor(2);
				}
			}
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("Rect")) {
			if(ImGui::Button("Add RectLight")) {
				if(rectLights_.size() < MaxCount) {
					AddLight(RECTLIGHT);
					selectRectLightIndex_ = static_cast<int>(rectLights_.size()) - 1;
				}
			}
			if(rectLights_.empty()) {
				ImGui::Text("No Rect Lights");
			}
			else {
				int maxIdx = static_cast<int>(rectLights_.size()) - 1;
				ImGui::SliderInt("Select", &selectRectLightIndex_, 0, maxIdx);
				ImGui::Separator();
				if(selectRectLightIndex_ >= 0 && selectRectLightIndex_ < (int)rectLights_.size()) {
					rectLights_[selectRectLightIndex_]->ImGui(selectRectLightIndex_);
					ImGui::Spacing();
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
					if(ImGui::Button("Delete This Light", ImVec2(-1, 0))) {
						rectLights_.erase(rectLights_.begin() + selectRectLightIndex_);
						if(selectRectLightIndex_ > 0) {
							selectRectLightIndex_--;
						}
					}
					ImGui::PopStyleColor(2);
				}
			}
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
#endif
}

void LightManager::AddLight(LightType type) {
	switch(type) {
	case DIRECTIONALLIGHT:
		dirLights_.push_back(std::make_unique<DirectionalLight>());
		lightCPUData_.count.dirLight++;
		break;
	case POINTLIGHT:
		pointLights_.push_back(std::make_unique<PointLight>());
		lightCPUData_.count.pointLight++;
		break;
	case SPOTLIGHT:
		spotLights_.push_back(std::make_unique<SpotLight>());
		lightCPUData_.count.spotLight++;
		break;
	case RECTLIGHT:
		rectLights_.push_back(std::make_unique<RectLight>());
		lightCPUData_.count.rectLight++;
		break;
	}
}

void LightManager::DeleteLight(LightType type, uint32_t index) {
	switch (type) {
	case DIRECTIONALLIGHT:
		if(index < dirLights_.size()) dirLights_.erase(dirLights_.begin() + index);
		break;
	case POINTLIGHT:
		if(index < pointLights_.size()) pointLights_.erase(pointLights_.begin() + index);
		break;
	case SPOTLIGHT:
		if(index < spotLights_.size()) spotLights_.erase(spotLights_.begin() + index);
		break;
	case RECTLIGHT:
		if(index < rectLights_.size()) rectLights_.erase(rectLights_.begin() + index);
		break;
	}
}