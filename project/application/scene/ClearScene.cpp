#include "ClearScene.h"
#include "MagosuyaEngine.h"
#include "Easing.h"

//デルタタイムを定義
const float kDeltaTime = 1.0f / 60.0f;

ClearScene::ClearScene (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) {
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

	//クリアスプライト
	clear_ = std::make_unique<Sprite> (dxCommon);
	TextureManager::GetInstance ()->LoadTexture ("Resources/UI/clear.png", "clear");
	
	camera_ = camera;
	input_ = inputManager;
}

ClearScene::~ClearScene () {
	
}

void ClearScene::Initialize () {
	nowScene_ = SceneLabel::Clear;
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

	clear_->SetID ("clear");
	clear_->Initialize ({ 340.0f, 260.0f, 0.0f });
	clear_->SetTexture ("clear");

	//カメラの現在地
	gameCameraPos_ = camera_->GetPosition ("FollowCamera");
	gameCameraRotate_ = camera_->GetRotate ("FollowCamera");

	//定点カメラ用のtransform
	camera_->AddCamera ("mainCamera3", CameraType::FixedPontCamera);
	camera_->SetActiveCamera ("mainCamera3");
	camera_->SetPosition (gameCameraPos_);
	camera_->SetRotate (gameCameraRotate_);

	t_ = 0.0f;

	bgm_.Initialize();
	bgmHandle_ = bgm_.LoadSound("resources/Audio/BGM/Clear.mp3");
	bgm_.PlaySoundW(bgmHandle_, true);
}

void ClearScene::Update () {
	// ゲーム終了
	if (input_->GetRawInput()->Trigger(VK_SPACE) || input_->GetGamePad()->TriggerButton(Button::A)) {
		nextScene_ = SceneLabel::Title;
		isFinish_ = true;
		bgm_.StopSound(bgmHandle_);
	}

	if (t_ <= 1.0f) {
		t_ += kDeltaTime / 20.0f;
	}
	if (t_ >= 0.99999f) {
		t_ = 1.0f;
	}

	ground_->Update (camera_->GetVPMatrix ());
	mountain_->Update (camera_->GetVPMatrix ());
	stone_->Update (camera_->GetVPMatrix ());
	skydome_->Update (camera_->GetVPMatrix ());
	clear_->Update ();

	camera_->SetPosition (Math::Lerp (camera_->GetPosition ("mainCamera3"), cameraGoalPos_, Easing::easeOutCirc (t_)));
	camera_->SetRotate(Math::Lerp (camera_->GetRotate ("mainCamera3"), cameraGoalRotate_, Easing::easeOutCirc (t_)));
	camera_->Update ();
}

void ClearScene::Draw () {
	skydome_->Draw ();
	ground_->Draw ();
	mountain_->Draw ();
	stone_->Draw ();
	if (t_ >= 1.0f) {
		clear_->Draw ();
	}
}