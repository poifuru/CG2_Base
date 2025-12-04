#include "PlayScene.h"
#include "MathFunction.h"
#include "CameraOrganizer.h"
#include "InputManager.h"

PlayScene::PlayScene (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) {
	player_ = std::make_unique<Player>(inputManager);
	boss_ = std::make_unique<Boss>();

	camera_ = camera;
	input_ = inputManager;
}

PlayScene::~PlayScene () {
	
}

void PlayScene::Initialize () {
	nowScene_ = SceneLabel::Play;
	isFinish_ = false;
	player_->Initialize();
	boss_->Initialize();

	Transform transform = {};
	camera_->AddCamera ("FollowCamera", CameraType::FollowCamera, transform);
	camera_->SetActiveCamera ("FollowCamera");
}

void PlayScene::Update () {
	boss_->ImGuiControl();

	if (input_->GetRawInput()->Trigger(VK_SPACE)) {
		nextScene_ = SceneLabel::Title;
		isFinish_ = true;
	}

	player_->Update(camera_->GetVPMatrix());
	boss_->Update(camera_->GetVPMatrix());
	camera_->SetFollowTarget ("FollowCamera", player_->GetTransform ());
}

void PlayScene::Draw () {
	player_->Draw();
	boss_->Draw();
}