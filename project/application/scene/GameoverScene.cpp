#include "GameoverScene.h"
#include "MagosuyaEngine.h"
#include "InputManager.h"
#include "Easing.h"
#include "SceneManager.h"
#include "SceneType.h"

//デルタタイムを定義
const float kDeltaTime = 1.0f / 60.0f;

GameoverScene::GameoverScene () {
	
}

GameoverScene::~GameoverScene () {

}

void GameoverScene::Initialize (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) {
	camera_ = camera;
	input_ = inputManager;

	camera_->AddCamera ("main3", CameraType::FixedPontCamera);
	camera_->SetActiveCamera("main3");
	camera_->SetPosition({ 0.0f, 100.0f, -10.0f });
}

void GameoverScene::Update () {
	if(input_->GetRawInput()->Trigger(VK_SPACE)) {
		nextScene_ = new TitleScene();
		SceneManager::GetInstance()->SetNextScene(nextScene_);
	}

	camera_->Update();
}

void GameoverScene::Draw () {

}

void GameoverScene::StopToResources() {

}
