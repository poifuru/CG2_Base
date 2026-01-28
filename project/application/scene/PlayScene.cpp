#include "PlayScene.h"
#include "MathFunction.h"
#include "CameraOrganizer.h"
#include "InputManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "SceneManager.h"
#include "SceneType.h"

PlayScene::PlayScene () {
	mapchip_ = std::make_unique<MapChip>();

	TextureManager::GetInstance()->LoadTexture("Resources/map/map1.png", "map1");
	TextureManager::GetInstance()->LoadTexture("Resources/map/map2.png", "map2");
	TextureManager::GetInstance()->LoadTexture("Resources/map/map3.png", "map3");
}

PlayScene::~PlayScene () {
	TextureManager::GetInstance()->UnloadTexture("Resources/map/map1.png");
	TextureManager::GetInstance()->UnloadTexture("Resources/map/map2.png");
	TextureManager::GetInstance()->UnloadTexture("Resources/map/map3.png");
}

void PlayScene::Initialize (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) {
	camera_ = camera;
	input_ = inputManager;

	//camera_->AddCamera("main2", CameraType::FixedPontCamera);

	lightManager_ = std::make_unique<LightManager>(dxCommon);
	lightManager_->Initialize();
	lightManager_->AddLight(LightType::DIRECTIONALLIGHT);

	mapchip_->Initialize(dxCommon, lightManager_.get());
	mapchip_->LoadMapChipCSV("Resources/map/mapData.csv");
}

void PlayScene::Update () {
	if(input_->GetRawInput()->Trigger(VK_F1)) {
		nextScene_ = new TitleScene();
		SceneManager::GetInstance()->SetNextScene(nextScene_);
	}
	camera_->Update();

	mapchip_->Update(camera_->GetVPMatrix(), camera_->GetPosition("Debug"));
	mapchip_->ImGui("mapchip");

	lightManager_->Update();
	lightManager_->ImGui();
}

void PlayScene::Draw () {
	mapchip_->Draw();
}