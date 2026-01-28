#include "PlayScene.h"
#include "MathFunction.h"
#include "CameraOrganizer.h"
#include "InputManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "SceneManager.h"
#include "SceneType.h"

PlayScene::PlayScene () {
	TextureManager::GetInstance()->LoadTexture("Resources/map/map1.png", "map1");
	TextureManager::GetInstance()->LoadTexture("Resources/map/map2.png", "map2");
	TextureManager::GetInstance()->LoadTexture("Resources/map/map3.png", "map3");
	ModelManager::GetInstance()->LoadModelData("Resources/player", "player.obj");
	TextureManager::GetInstance()->LoadTexture("Resources/player/player.png", "player");
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
	for(int i = 0; i < 5; ++i) {
		lightManager_->AddLight(LightType::DIRECTIONALLIGHT);
		lightManager_->SetDirectionalLightIntensity(i, 2.0f);
	}
	lightManager_->SetDirectionalLightDir(1, { 0.0f, 1.0f, 0.0f });
	lightManager_->SetDirectionalLightDir(2, { 1.0f, 0.0f, 0.0f });
	lightManager_->SetDirectionalLightDir(3, { 0.0f, -1.0f, 0.0f });
	lightManager_->SetDirectionalLightDir(4, { -1.0f, 0.0f, 0.0f });

	//オブジェクトたちの初期化
	mapchip_ = std::make_unique<MapChip>();
	mapchip_->Initialize(dxCommon, lightManager_.get());
	mapchip_->LoadMapChipCSV("Resources/map/mapData.csv");

	player_ = std::make_unique<Player>(dxCommon, camera, inputManager, lightManager_.get(), mapchip_.get());
	player_->Initialize();
}

void PlayScene::Update () {
	if(input_->GetRawInput()->Trigger(VK_F1)) {
		nextScene_ = new TitleScene();
		SceneManager::GetInstance()->SetNextScene(nextScene_);
	}
	camera_->Update();
	lightManager_->Update();
	lightManager_->ImGui();

	mapchip_->Update(camera_->GetVPMatrix(), camera_->GetPosition("Debug"));
	mapchip_->ImGui("mapchip");

	player_->Update();
	player_->ImGui();
	
}

void PlayScene::Draw () {
	mapchip_->Draw();
	player_->Draw();
}