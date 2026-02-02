#include "ClearScene.h"
#include "Easing.h"
#include "ModelManager.h"
#include "SceneManager.h"
#include "SceneType.h"

//デルタタイムを定義
const float kDeltaTime = 1.0f / 60.0f;

ClearScene::ClearScene () {
	
}

ClearScene::~ClearScene () {

}

void ClearScene::Initialize (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) {
	camera_ = camera;
	input_ = inputManager;

	camera_->AddCamera ("main2", CameraType::FixedPontCamera);
	camera_->SetActiveCamera("main2");
	camera_->SetPosition({ 0.0f, 100.0f, -10.0f });
}

void ClearScene::Update () {
	if(input_->GetRawInput()->Trigger(VK_SPACE)) {
		nextScene_ = new TitleScene();
		SceneManager::GetInstance()->SetNextScene(nextScene_);
	}

	camera_->Update();
}

void ClearScene::Draw () {

}

void ClearScene::StopToResources() {

}
