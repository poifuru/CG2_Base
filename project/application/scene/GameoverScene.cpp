#include "GameoverScene.h"
#include "MagosuyaEngine.h"
#include "InputManager.h"
#include "Easing.h"

//デルタタイムを定義
const float kDeltaTime = 1.0f / 60.0f;

GameoverScene::GameoverScene (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) {
	camera_ = camera;
	input_ = inputManager;
}

GameoverScene::~GameoverScene () {

}

void GameoverScene::Initialize () {
	nextScene_ = SceneLabel::Title;
	isFinish_ = true;
}

void GameoverScene::Update () {

}

void GameoverScene::Draw () {

}
