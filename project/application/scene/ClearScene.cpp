#include "ClearScene.h"
#include "Easing.h"
#include "ModelManager.h"

//デルタタイムを定義
const float kDeltaTime = 1.0f / 60.0f;

ClearScene::ClearScene (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) {
	camera_ = camera;
	input_ = inputManager;

	clear_ = std::make_unique<Model>(dxCommon);
	ModelManager::GetInstance()->LoadModelData("Resources/clear", "clear");

	camera_->AddCamera("main2", CameraType::FixedPontCamera);
}

ClearScene::~ClearScene () {

}

void ClearScene::Initialize () {
	nowScene_ = SceneLabel::Clear;
	isFinish_ = false;

	clear_->SetModelData("clear");
	clear_->SetTexture("Dummy");
	clear_->Initialize();
	clear_->IsLighting(LightReflectionModel::None);
	clear_->SetColor({ 1.0f, 0.80f, 0.0f, 1.0f });
	clear_->SetPosition({ 0.0f, 100.0f, 0.0f });

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

	camera_->SetActiveCamera("main2");
	camera_->SetPosition({ 0.0f, 100.0f, -10.0f });
}

void ClearScene::Update () {
	if(input_->GetRawInput()->Trigger(VK_SPACE)) {
		nextScene_ = SceneLabel::Title;
		isFinish_ = true;
	}

	clear_->Update(&camera_->GetVPMatrix());

	camera_->Update();

	//光源のdirectionの正規化
	directionalLightData_->direction = Math::Normalize(directionalLightData_->direction);
}

void ClearScene::Draw () {
	clear_->Draw(dierctionalLightResource_.Get());
}