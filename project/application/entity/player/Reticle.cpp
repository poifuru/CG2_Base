#include "Reticle.h"

Reticle::Reticle(DxCommon* dxCommon, CameraOrganizer* camera, InputManager* input, LightManager* light) {
	model_ = std::make_unique<Model>(dxCommon, light);
	model_->IsLighting(LightReflectionModel::None);
}

Reticle::~Reticle() {

}

void Reticle::Initialize() {
	model_->SetModelData("reticle.obj");
	model_->SetTexture("reticle");
	model_->Initialize();


}

void Reticle::Update() {

}

void Reticle::Draw() {

}

void Reticle::ImGui() {

}
