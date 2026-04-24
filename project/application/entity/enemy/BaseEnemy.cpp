#include "BaseEnemy.h"

BaseEnemy::BaseEnemy(DxCommon* dxCommon, LightManager* light) {
	model_ = std::make_unique<Model>(dxCommon, light);
}

BaseEnemy::~BaseEnemy() {

}

void BaseEnemy::Initialize() {
	model_->Initialize();
}

void BaseEnemy::Update() {

}

void BaseEnemy::Draw() {
	model_->Draw();
}

void BaseEnemy::ImGui() {
	model_->ImGui("enemy");
}