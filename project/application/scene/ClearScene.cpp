#include "ClearScene.h"
#include "MagosuyaEngine.h"
#include "Easing.h"

//デルタタイムを定義
const float kDeltaTime = 1.0f / 60.0f;

ClearScene::ClearScene (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) {
	camera_ = camera;
	input_ = inputManager;
}

ClearScene::~ClearScene () {

}

void ClearScene::Initialize () {
	nowScene_ = SceneLabel::Clear;
	isFinish_ = false;
}

void ClearScene::Update () {

}

void ClearScene::Draw () {
}