#include "GameoverScene.h"
#include "MagosuyaEngine.h"
#include "InputManager.h"

GameoverScene::GameoverScene (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) {
	camera_ = camera;
	input_ = inputManager;
}

GameoverScene::~GameoverScene () {
	
}

void GameoverScene::Initialize () {
	nowScene_ = SceneLabel::Gameover;
	isFinish_ = false;
}

void GameoverScene::Update () {
	
}

void GameoverScene::Draw () {
	
}
