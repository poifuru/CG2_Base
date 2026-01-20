#include "TitleScene.h"
#include "CameraOrganizer.h"
#include "InputManager.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "Easing.h"
#include "imgui.h"

//デルタタイムを定義
const float kDeltaTime = 1.0f / 60.0f;

TitleScene::TitleScene(CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) {
	camera_ = camera;
	input_ = inputManager;

	lightManager_ = std::make_unique<LightManager>(dxCommon);

	sphere_ = std::make_unique<SphereMesh>(dxCommon, lightManager_.get());
	terrain_ = std::make_unique<Model>(dxCommon, lightManager_.get());
	TextureManager::GetInstance()->LoadTexture("Resources/monsterBall.png", "monsterBall");
	ModelManager::GetInstance()->LoadModelData("Resources/terrain", "terrain");
}

TitleScene::~TitleScene () {
	
}

void TitleScene::Initialize () {
	nowScene_ = SceneLabel::Title;
	isFinish_ = false;

	lightManager_->Initialize();
	lightManager_->AddLight(LightType::DIRECTIONALLIGHT);
	lightManager_->AddLight(LightType::POINTLIGHT);
	lightManager_->AddLight(LightType::SPOTLIGHT);

	sphere_->Initialize({0.0f, 0.0f, 0.0f}, 3.0f);

	terrain_->SetModelData("terrain");
	terrain_->SetTexture("terrain");
	terrain_->Initialize();
}

void TitleScene::Update () {
	camera_->Update();

	sphere_->Update(&camera_->GetCameraData());
	sphere_->ImGui();

	terrain_->Update(&camera_->GetCameraData());
	terrain_->ImGui("terrain");

	lightManager_->Update();
	lightManager_->ImGui();
}

void TitleScene::Draw () {
	sphere_->Draw(TextureManager::GetInstance()->GetTextureHandle("monsterBall"));
	terrain_->Draw();
}