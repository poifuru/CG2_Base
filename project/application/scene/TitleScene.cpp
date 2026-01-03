#include "TitleScene.h"
#include "CameraOrganizer.h"
#include "InputManager.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "Easing.h"

//デルタタイムを定義
const float kDeltaTime = 1.0f / 60.0f;

TitleScene::TitleScene (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) {
	camera_ = camera;
	input_ = inputManager;

	//モデルのロード
	model_ = std::make_unique<Model>(dxCommon);
	ModelManager::GetInstance()->LoadModelData("Resources/teapot", "teapot");
}

TitleScene::~TitleScene () {
	
}

void TitleScene::Initialize () {
	nowScene_ = SceneLabel::Title;
	isFinish_ = false;

	model_->SetModelData("teapot");
	model_->SetTexture("teapot");
	model_->Initialize();
	model_->IsLighting(LightReflectionModel::HalfLambert);
}

void TitleScene::Update () {
	camera_->Update();
	model_->Update(&camera_->GetVPMatrix());
	model_->ImGui("teapot");
}

void TitleScene::Draw () {
	model_->Draw();
}