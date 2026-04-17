#include "PlayScene.h"
#include "MathFunction.h"
#include "CameraOrganizer.h"
#include "InputManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "SceneManager.h"
#include "SceneType.h"

PlayScene::PlayScene () {
	ModelManager::GetInstance()->LoadModelData("Resources/skydome", "skydome.obj");
	TextureManager::GetInstance()->LoadTexture("Resources/uvChecker.png", "uvChecker");
}

PlayScene::~PlayScene () {
}

void PlayScene::Initialize (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) {
	camera_ = camera;
	input_ = inputManager;
	dxCommon_ = dxCommon;

	camera_->AddCamera("main2", CameraType::FollowCamera);
	camera_->SetActiveCamera("main2");

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
	skydome_ = std::make_unique<Model>(dxCommon, lightManager_.get());
	skydome_->SetModelData("skydome.obj");
	skydome_->SetTexture("uvChecker");
	skydome_->Initialize();
	skydome_->IsLighting(LightReflectionModel::None);

	player_ = std::make_unique<Player>(dxCommon_, camera_, input_, lightManager_.get());
	player_->Initialize();
}

void PlayScene::Update () {
	if(input_->GetRawInput()->Trigger(VK_F1)) {
		nextScene_ = std::make_unique<TitleScene>();
		SceneManager::GetInstance()->SetNextScene(std::move(nextScene_));
	}
	lightManager_->Update();
	lightManager_->ImGui();

	player_->Update();

	camera_->SetFollowTarget("main2", player_->GetTransform());
	camera_->Update();
	camera_->ImGui();

	skydome_->Update(&camera_->GetCameraData());
}
void PlayScene::Draw () {
	skydome_->Draw();
	player_->Draw();
}

void PlayScene::StopToResources() {

}