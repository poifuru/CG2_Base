#include "PlayScene.h"
#include "MathFunction.h"
#include "CameraOrganizer.h"
#include "InputManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "SceneManager.h"
#include "SceneType.h"

PlayScene::PlayScene () {
	
}

PlayScene::~PlayScene () {

}

void PlayScene::Initialize (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) {
	camera_ = camera;
	input_ = inputManager;
}

void PlayScene::Update () {
	if(input_->GetRawInput()->Trigger(VK_F1)) {
		nextScene_ = new TitleScene();
		SceneManager::GetInstance()->SetNextScene(nextScene_);
	}
	camera_->Update();
}

void PlayScene::Draw () {
}