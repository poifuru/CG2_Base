#include "PCH.h"
//#include "BaseEnemy.h"
//
//BaseEnemy::BaseEnemy(DxCommon* dxCommon, LightManager* light, CameraOrganizer* camera) {
//	model_ = std::make_unique<Model>(dxCommon, light);
//	camera_ = camera;
//}
//
//BaseEnemy::~BaseEnemy() {
//
//}
//
//void BaseEnemy::Initialize() {
//	model_->Initialize();
//}
//
//void BaseEnemy::Update() {
//
//}
//
//void BaseEnemy::Draw() {
//	model_->Draw();
//}
//
//void BaseEnemy::ImGui() {
//	model_->ImGui("enemy");
//}
//
//void BaseEnemy::SetAlpha(float alpha) {
//	if (model_) {
//		model_->SetAlpha(alpha);
//	}
//}
//
//void BaseEnemy::SetScale(const Vector3& scale) {
//	if (model_) {
//		model_->SetScale(scale);
//	}
//}