#include "LightManager.h"
#include "SRVManager.h"

LightManager::LightManager() {
}

LightManager::~LightManager() {
}

void LightManager::Initialize() {

}

void LightManager::Update() {
	//各ライトを更新（さっき作ったImGuiの処理などが走るでやんす）
	for(auto& light : dirLights_) {
		light->Update();
	}

	//データを集めてMap済みのメモリに書き込む
	for(size_t i = 0; i < dirLights_.size(); ++i) {
		dirLightData_[i].color = dirLights_[i]->GetColor();
		dirLightData_[i].direction = dirLights_[i]->GetDirection();
		dirLightData_[i].intensity = dirLights_[i]->GetIntensity();
	}
}

void LightManager::AddLight(LightType type) {
	switch(type) {
	case DIRECTIONALLIGHT:
		dirLights_.push_back(std::make_unique<DirectionalLight>());
		break;

	case POINTLIGHT:

		break;

	case SPOTLIGHT:

		break;

	default:
		break;
	}
}