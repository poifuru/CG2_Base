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

	//CameraOrganizer::GetInstance ()->ImGui ();
}

void SceneManager::Draw () {
	currentScene_->Draw ();
}