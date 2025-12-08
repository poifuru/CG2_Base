#include "ClearScene.h"
#include "MagosuyaEngine.h"

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