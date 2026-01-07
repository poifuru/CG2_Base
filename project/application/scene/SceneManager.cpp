#include "SceneManager.h"
#include "CameraOrganizer.h"
#include "ModelManager.h"

SceneManager::SceneManager (CameraOrganizer* camera, InputManager* input, DxCommon* dxCommon) {
	scene_ = SceneLabel::Title;
	titleScene_ = std::make_unique<TitleScene> (camera, input, dxCommon);
	playScene_ = std::make_unique<PlayScene> (camera, input, dxCommon);
	clearScene_ = std::make_unique<ClearScene> (camera, input, dxCommon);
	gameoverScene_ = std::make_unique<GameoverScene> (camera, input, dxCommon);
	currentScene_ = titleScene_.get ();

	skydome_ = std::make_unique<Model>(dxCommon);
	ModelManager::GetInstance()->LoadModelData("Resources/skydome", "skydome");

	camera_ = camera;
}

SceneManager::~SceneManager () {

}

void SceneManager::Initialize (SceneLabel scene) {
	//引数で初期化のシーンを選択
	switch (scene) {
	case SceneLabel::Title:
		currentScene_ = titleScene_.get ();
		break;

	case SceneLabel::Play:
		currentScene_ = playScene_.get ();
		break;

	case SceneLabel::Clear:
		currentScene_ = clearScene_.get ();
		break;

	case SceneLabel::Gameover:
		currentScene_ = gameoverScene_.get ();
		break;
	}
	currentScene_->Initialize ();

	skydome_->SetModelData("skydome");
	skydome_->SetTexture("skydome");
	skydome_->Initialize();
	skydome_->IsLighting(LightReflectionModel::HalfLambert);

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

void SceneManager::Update () {
	currentScene_->Update ();

	if (currentScene_->GetIsFinish ()) {
		switch (currentScene_->GetNextScene ()) {
		case SceneLabel::Title:
			currentScene_ = titleScene_.get ();
			break;

		case SceneLabel::Play:
			currentScene_ = playScene_.get ();
			break;

		case SceneLabel::Clear:
			currentScene_ = clearScene_.get ();
			break;

		case SceneLabel::Gameover:
			currentScene_ = gameoverScene_.get ();
			break;
		}
		currentScene_->Initialize ();
	}

	skydome_->Update(&camera_->GetVPMatrix());
	rotate_ -= 0.001f;

	skydome_->SetRotate({ 0.0f, rotate_, 0.0f });

	//光源のdirectionの正規化
	directionalLightData_->direction = Math::Normalize(directionalLightData_->direction);

	//CameraOrganizer::GetInstance ()->ImGui ();
}

void SceneManager::Draw () {
	skydome_->Draw(dierctionalLightResource_.Get());
	currentScene_->Draw ();
}