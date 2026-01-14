#include "ClearScene.h"
#include "Easing.h"
#include "ModelManager.h"

//デルタタイムを定義
const float kDeltaTime = 1.0f / 60.0f;

ClearScene::ClearScene (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) {
	camera_ = camera;
	input_ = inputManager;

	camera_->AddCamera("main2", CameraType::FixedPontCamera);
}

ClearScene::~ClearScene () {

}

void ClearScene::Initialize () {
	nowScene_ = SceneLabel::Clear;
	isFinish_ = false;

	camera_->SetActiveCamera("main2");
	camera_->SetPosition({ 0.0f, 100.0f, -10.0f });
}

void ClearScene::Update () {
	if(input_->GetRawInput()->Trigger(VK_SPACE)) {
		nextScene_ = SceneLabel::Title;
		isFinish_ = true;
	}

	camera_->Update();
}

void ClearScene::Draw () {
}