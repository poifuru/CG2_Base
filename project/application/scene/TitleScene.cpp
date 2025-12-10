#include "TitleScene.h"
#include "CameraOrganizer.h"
#include "InputManager.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "Easing.h"

//デルタタイムを定義
const float kDeltaTime = 1.0f / 60.0f;

TitleScene::TitleScene (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) {
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

	//タイトル
	moji_ = std::make_unique<Model> (dxCommon);
	ModelManager::GetInstance ()->LoadModelData ("Resources/Title/moji", "moji");

	zako_ = std::make_unique<Model> (dxCommon);
	ModelManager::GetInstance ()->LoadModelData ("Resources/Title/zako", "zako2");

	pushA_ = std::make_unique<Sprite> (dxCommon);
	TextureManager::GetInstance ()->LoadTexture ("Resources/UI/pushA.png", "pushA");

	particle_ = std::make_unique<MeshParticle> ();

	camera_ = camera;
	input_ = inputManager;
}

TitleScene::~TitleScene () {
	audio_.Unload(bgmHandle_);
	audio_.Unload(clickHandle_);
}

void TitleScene::Initialize () {
	nowScene_ = SceneLabel::Title;
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

	//タイトル
	moji_->SetModelData ("moji");
	moji_->SetTexture ("moji");
	moji_->Initialize ();
	moji_->IsLighting (LightReflectionModel::HalfLambert);
	moji_->SetTransform ({
		{0.25f, 0.25f, 0.25f},
		{-0.19f, -1.39f, 0.21f},
		{6.63f, 13.37f, -0.16f},
						  });
	mojiPos = moji_->GetPosition ();

	zako_->SetModelData ("zako2");
	zako_->SetTexture ("zako2");
	zako_->Initialize ();
	zako_->IsLighting (LightReflectionModel::HalfLambert);
	zako_->SetTransform ({
		{0.25f, 0.25f, 0.25f},
		{-0.19f, -1.39f, 0.21f},
		{6.63f, 13.37f, -0.16f},
						 });
	zakoPos = zako_->GetPosition ();

	pushA_->SetID ("pushA");
	pushA_->Initialize ({ 400.0f, 500.0f, 0.0f });
	pushA_->SetTexture ("pushA");

	particle_->Initialize ();
	particle_->SetEmitterPos ({ 46.5f, 21.0f, 59.0f });
	particle_->SetColor ({ 1.0f, 1.0f, 1.0f, 1.0f });

	//定点カメラ用のtransform
	camera_->AddCamera ("mainCamera1", CameraType::FixedPontCamera);
	camera_->SetActiveCamera ("mainCamera1");
	camera_->SetPosition ({ 0.0f, 0.0f, -50.0f });
	camera_->SetRotate ({ -0.17f, 0.23f, 0.0f });

	t_ = 0.0f;

	audio_.Initialize();
	bgmHandle_ = audio_.LoadSound("resources/Audio/BGM/Title.mp3");
	clickHandle_ = audio_.LoadSound("resources/Audio/SE/click.mp3");
	audio_.PlaySoundW(bgmHandle_, 0.5f, true);
}

void TitleScene::Update () {
	// ゲーム終了
	if (t_ >= 1.0f && input_->GetRawInput()->Trigger(VK_SPACE) || input_->GetGamePad()->TriggerButton(Button::A)) {
		nextScene_ = SceneLabel::Play;
		isFinish_ = true;
		audio_.PlaySoundW(clickHandle_, 1.0f, false);
		audio_.StopSound(bgmHandle_);
	}

	if (t_ <= 1.0f) {
		t_ += kDeltaTime / 3.0f;
	}

	ground_->Update (camera_->GetVPMatrix ());
	mountain_->Update (camera_->GetVPMatrix ());
	stone_->Update (camera_->GetVPMatrix ());
	skydome_->Update (camera_->GetVPMatrix ());
	moji_->Update (camera_->GetVPMatrix ());
	moji_->SetPosition ({ mojiPos.x, Math::Lerp (40.0f, mojiPos.y, Easing::easeOutBounce (t_)), mojiPos.z });
	moji_->ImGui ("moji");
	zako_->Update (camera_->GetVPMatrix ());
	zako_->SetPosition ({ zakoPos.x, Math::Lerp (40.0f, zakoPos.y, Easing::easeOutBounce (t_)), zakoPos.z });
	zako_->ImGui ("zako2");
	pushA_->Update ();

	particleTimeCount_ += kDeltaTime;
	if (particleTimeCount_ >= particleTimer_) {
		particle_->Spawn ();
		particleTimeCount_ = 0.0f;
	}

	particle_->Update (camera_->GetVPMatrix ());

	camera_->Update ();
}

void TitleScene::Draw () {
	skydome_->Draw ();
	ground_->Draw ();
	mountain_->Draw ();
	stone_->Draw ();
	moji_->Draw ();
	zako_->Draw ();
	if (t_ >= 1.0f) {
		pushA_->Draw ();
	}
	particle_->Draw ();
}