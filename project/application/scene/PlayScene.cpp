#include "PlayScene.h"
#include "MathFunction.h"
#include "CameraOrganizer.h"
#include "InputManager.h"
#include "TextureManager.h"
#include "ModelManager.h"

PlayScene::PlayScene (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) {
	camera_ = camera;
	input_ = inputManager;

	map_ = std::make_unique<Model>(dxCommon);
	ModelManager::GetInstance()->LoadModelData("Resources/map", "map");
}

PlayScene::~PlayScene () {

}

void PlayScene::Initialize () {
	nowScene_ = SceneLabel::Play;
	isFinish_ = false;

	map_->SetModelData("map");
	map_->SetTexture("map");
	map_->Initialize();
	map_->IsLighting(LightReflectionModel::HalfLambert);

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

void PlayScene::Update () {
	if(input_->GetRawInput()->Trigger(VK_F1)) {
		nextScene_ = SceneLabel::Title;
		isFinish_ = true;
	}
	camera_->Update();

	map_->Update(&camera_->GetVPMatrix());

	directionalLightData_->direction = Math::Normalize(directionalLightData_->direction);
}

void PlayScene::Draw () {
	map_->Draw(dierctionalLightResource_.Get());
}