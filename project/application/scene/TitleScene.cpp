#include "TitleScene.h"
#include "CameraOrganizer.h"
#include "InputManager.h"
#include "ModelManager.h"
#include "TextureManager.h"

TitleScene::TitleScene (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) {
	camera_ = camera;
	input_ = inputManager;
	model_ = std::make_unique<Model> (dxCommon);
	ModelManager::GetInstance()->LoadModelData ("Resources/teapot", "teapot");
}

TitleScene::~TitleScene () {

}

void TitleScene::Initialize () {
	nowScene_ = SceneLabel::Title;
	isFinish_ = false;

	model_->SetModelData ("teapot");
	model_->SetTexture ("teapot");
	model_->Initialize ();

	//定点カメラ用のtransform
	Transform transform = {{1.0f, 1.0f, 1.0f},{},{0.0f, 0.0f, -50.0f},};
	camera_->AddCamera ("mainCamera1", CameraType::FixedPontCamera, transform);
	camera_->SetActiveCamera ("mainCamera1");
}

void TitleScene::Update () {
	// ゲーム終了
	if (input_->GetRawInput()->Trigger(VK_F1)) {
		nextScene_ = SceneLabel::Play;
		isFinish_ = true;
	}

	model_->Update (camera_->GetVPMatrix());
}

void TitleScene::Draw () {
	model_->Draw ();
}