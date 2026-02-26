#include "TestScene.h"
#include "ModelManager.h"
#include "TextureManager.h"

TestScene::TestScene() {
	ModelManager::GetInstance()->LoadModelData("Resources/AnimatedCube", "AnimatedCube.gltf");
	ModelManager::GetInstance()->LoadAnimationData("Resources/AnimatedCube", "AnimatedCube.gltf");
	TextureManager::GetInstance()->TextureManager::LoadTexture(
		"Resources/AnimatedCube/AnimatedCube_BaseColor.png", "Cube"
	);

}

TestScene::~TestScene() {

}

void TestScene::Initialize(CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) {
	camera_ = camera;
	input_ = inputManager;

	lightManager_ = std::make_unique<LightManager>(dxCommon);
	lightManager_->Initialize();
	lightManager_->AddLight(LightType::POINTLIGHT);

	cube_ = std::make_unique<Model>(dxCommon, lightManager_.get());
	cube_->SetModelData("AnimatedCube.gltf");
	cube_->SetTexture("Cube");
	cube_->SetAnimation("AnimatedCube.gltf");
	cube_->Initialize({ 1.0f, 1.0f, 1.0f }, {}, { -5.0f, 0.0f, 0.0f });

	human_ = std::make_unique<Model>(dxCommon, lightManager_.get());

}

void TestScene::Update() {
	camera_->Update();

	lightManager_->Update();
	lightManager_->ImGui();

	cube_->Update(&camera_->GetCameraData());
	cube_->ImGui("AnimatedCube");
}

void TestScene::Draw() {
	cube_->Draw();
}

void TestScene::StopToResources() {

}