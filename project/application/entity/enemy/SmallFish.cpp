#include "PCH.h"
//#include "SmallFish.h"
//
//SmallFish::SmallFish(DxCommon* dxCommon, LightManager* light, CameraOrganizer* camera)
//	: BaseEnemy(dxCommon, light, camera) {
//}
//
//SmallFish::~SmallFish() {
//
//}
//
//void SmallFish::Initialize() {
//	model_->SetModelData("teapot.obj");
//	model_->SetTexture("teapot");
//	BaseEnemy::Initialize();
//	aabbSize_ = { 1.0f, 1.0f, 1.0f };
//}
//
//void SmallFish::Update() {
//	model_->SetPosition(transform_.translate);
//	model_->Update(&camera_->GetCameraData());
//	// AABBの更新
//	aabb_.min = transform_.translate - aabbSize_;
//	aabb_.max = transform_.translate + aabbSize_;
//}
//
//void SmallFish::Draw() {
//	BaseEnemy::Draw();
//}
//
//void SmallFish::ImGui() {
//
//}
