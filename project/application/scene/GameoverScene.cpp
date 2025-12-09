#include "GameoverScene.h"
#include "MagosuyaEngine.h"
#include "InputManager.h"

GameoverScene::GameoverScene (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) {
	//地面のモデル
	ground_ = std::make_unique<Model> (dxCommon);
	ModelManager::GetInstance ()->LoadModelData ("Resources/field/ground", "ground");

	//火山
	mountain_ = std::make_unique<Model> (dxCommon);
	ModelManager::GetInstance ()->LoadModelData ("Resources/field/mountain", "mountain");

	//岩の壁
	stone_ = std::make_unique<Model> (dxCommon);
	ModelManager::GetInstance ()->LoadModelData ("Resources/field/stone", "stone");

	//天球
	skydome_ = std::make_unique<Model> (dxCommon);
	ModelManager::GetInstance ()->LoadModelData ("Resources/skydome", "skydome");
	
	camera_ = camera;
	input_ = inputManager;
}

GameoverScene::~GameoverScene () {
}

void GameoverScene::Initialize () {
	nowScene_ = SceneLabel::Gameover;
	isFinish_ = false;

	//地面のモデル
	ground_->SetModelData ("ground");
	ground_->SetTexture ("ground");
	ground_->Initialize ({ 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f });
	ground_->IsLighting (LightReflectionModel::HalfLambert);

	//火山
	mountain_->SetModelData ("mountain");
	mountain_->SetTexture ("mountain");
	mountain_->Initialize ({ 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, -1.2f, 0.0f });
	mountain_->IsLighting (LightReflectionModel::HalfLambert);

	//岩
	stone_->SetModelData ("stone");
	stone_->SetTexture ("stone");
	stone_->Initialize ({ 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, -1.2f, 0.0f });
	stone_->IsLighting (LightReflectionModel::HalfLambert);
	//岩の色
	Vector4 stoneColor = { 0.33f, 0.33f, 0.33f, 1.0f };
	stone_->SetColor (stoneColor);

	//天球
	skydome_->SetModelData ("skydome");
	skydome_->SetTexture ("skydome");
	skydome_->Initialize ();
	skydome_->IsLighting (LightReflectionModel::HalfLambert);

	//定点カメラ用のtransform
	camera_->AddCamera ("mainCamera2", CameraType::FixedPontCamera);
	camera_->SetActiveCamera ("mainCamera2");
	camera_->SetPosition ({ 0.0f, 0.0f, -50.0f });
}

void GameoverScene::Update () {
	ground_->Update (camera_->GetVPMatrix ());
	mountain_->Update (camera_->GetVPMatrix ());
	stone_->Update (camera_->GetVPMatrix ());
	skydome_->Update (camera_->GetVPMatrix ());

	camera_->Update ();
}

void GameoverScene::Draw () {
	skydome_->Draw ();
	ground_->Draw ();
	mountain_->Draw ();
	stone_->Draw ();
}
