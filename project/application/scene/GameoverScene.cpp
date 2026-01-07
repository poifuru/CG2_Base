#include "GameoverScene.h"
#include "MagosuyaEngine.h"
#include "InputManager.h"
#include "Easing.h"

//デルタタイムを定義
const float kDeltaTime = 1.0f / 60.0f;

GameoverScene::GameoverScene (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) {
	camera_ = camera;
	input_ = inputManager;

	gameover_ = std::make_unique<Model>(dxCommon);
	ModelManager::GetInstance()->LoadModelData("Resources/gameover", "gameover");

	camera_->AddCamera("main3", CameraType::FixedPontCamera);
}

GameoverScene::~GameoverScene () {

}

void GameoverScene::Initialize () {
	nextScene_ = SceneLabel::Title;
	isFinish_ = false;

	gameover_->SetModelData("gameover");
	gameover_->SetTexture("Dummy");
	gameover_->Initialize();
	gameover_->IsLighting(LightReflectionModel::None);
	gameover_->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f });
	gameover_->SetPosition({ 0.0f, 100.0f, 0.0f });

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

	camera_->SetActiveCamera("main3");
	camera_->SetPosition({ 0.0f, 100.0f, -10.0f });
}

void GameoverScene::Update () {
	if(input_->GetRawInput()->Trigger(VK_SPACE)) {
		nextScene_ = SceneLabel::Title;
		isFinish_ = true;
	}

	gameover_->Update(&camera_->GetVPMatrix());

	camera_->Update();

	//光源のdirectionの正規化
	directionalLightData_->direction = Math::Normalize(directionalLightData_->direction);
}

void GameoverScene::Draw () {
	gameover_->Draw(dierctionalLightResource_.Get());
}
