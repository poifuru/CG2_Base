#include "TitleScene.h"
#include "CameraOrganizer.h"
#include "InputManager.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "Easing.h"
#include "imgui.h"
#include "SceneManager.h"
#include "SceneType.h"

//デルタタイムを定義
const float kDeltaTime = 1.0f / 60.0f;

TitleScene::TitleScene() {
	TextureManager::GetInstance()->LoadTexture("Resources/monsterBall.png", "monsterBall");
	ModelManager::GetInstance()->LoadModelData("Resources/terrain", "terrain.obj");
	ModelManager::GetInstance()->LoadModelData("Resources/plane", "plane.gltf");
}

TitleScene::~TitleScene () {
	//リソースを解放する
	TextureManager::GetInstance()->UnloadTexture("Resources/monsterBall.png");
}

void TitleScene::Initialize (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) {
	camera_ = camera;
	input_ = inputManager;

	lightManager_ = std::make_unique<LightManager>(dxCommon);

	lightManager_->Initialize();
	lightManager_->AddLight(LightType::DIRECTIONALLIGHT);
	lightManager_->AddLight(LightType::POINTLIGHT);
	lightManager_->AddLight(LightType::SPOTLIGHT);

	sphere_ = std::make_unique<SphereMesh>(dxCommon, lightManager_.get());
	sphere_->Initialize({0.0f, 0.0f, 0.0f}, 3.0f);

	terrain_ = std::make_unique<Model>(dxCommon, lightManager_.get());
	terrain_->SetModelData("terrain");
	terrain_->SetTexture("terrain");
	terrain_->Initialize();

	plane_ = std::make_unique<Model>(dxCommon, lightManager_.get());
	plane_->SetModelData("plane");
	plane_->SetTexture("plane");
	plane_->Initialize();
}

void TitleScene::Update () {
	if(input_->GetRawInput()->Trigger(VK_F1)) {
		nextScene_ = new PlayScene();
		SceneManager::GetInstance()->SetNextScene(nextScene_);
	}

	camera_->Update();

	sphere_->Update(&camera_->GetCameraData());
	sphere_->ImGui();

	terrain_->Update(&camera_->GetCameraData());
	terrain_->ImGui("terrain");

	plane_->Update(&camera_->GetCameraData());
	plane_->ImGui("plane");

	lightManager_->Update();
	lightManager_->ImGui();
}

void TitleScene::Draw () {
	//sphere_->Draw(TextureManager::GetInstance()->GetTextureHandle("monsterBall"));
	//terrain_->Draw();
	plane_->Draw();
}