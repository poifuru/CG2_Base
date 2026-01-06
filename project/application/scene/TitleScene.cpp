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
	title_ = std::make_unique<Model>(dxCommon);
	ModelManager::GetInstance()->LoadModelData("Resources/pikopiko", "pikopiko");

	camera_->AddCamera("main1", CameraType::FixedPontCamera);
	camera_->SetActiveCamera("main1");
}

TitleScene::~TitleScene () {
	
}

void TitleScene::Initialize () {
	nowScene_ = SceneLabel::Title;
	isFinish_ = false;

	title_->SetModelData("pikopiko");
	title_->SetTexture("Dummy");
	title_->Initialize();
	title_->IsLighting(LightReflectionModel::HalfLambert);
	title_->SetColor({ 1.0f, 0.80f, 0.0f, 1.0f });

	dierctionalLightResource_ = DxCommon::GetInstance()->CreateBufferResource(sizeof(DirectionalLight));

	dierctionalLightResource_ = DxCommon::GetInstance()->CreateBufferResource(sizeof(DirectionalLight));
	//書き込むためのアドレス取得
	dierctionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	//実際に書き込み
	directionalLightData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData_->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData_->intensity = 1.0f;
	//ライティング用の変数
	colorLight = { 1.0f, 1.0f, 1.0f, 1.0f };
}

void TitleScene::Update () {
	if(input_->GetRawInput()->Trigger(VK_SPACE)) {
		nextScene_ = SceneLabel::Play;
		isFinish_ = true;
	}

	camera_->Update();
	
	title_->Update(&camera_->GetVPMatrix());
	title_->ImGui("pikopiko");

	//光源のdirectionの正規化
	directionalLightData_->direction = Math::Normalize (directionalLightData_->direction); 
}

void TitleScene::Draw () {
	title_->Draw(dierctionalLightResource_.Get());
}