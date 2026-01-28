#include "TitleScene.h"
#include "CameraOrganizer.h"
#include "InputManager.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "Easing.h"
#include "imgui.h"
#include "SceneManager.h"
#include "SceneType.h"
#include "AudioManager.h"

//デルタタイムを定義
const float kDeltaTime = 1.0f / 60.0f;

TitleScene::TitleScene() {
	TextureManager::GetInstance()->LoadTexture("Resources/monsterBall.png", "monsterBall");
	TextureManager::GetInstance()->LoadTexture("Resources/uvChecker.png", "uvChecker");
	TextureManager::GetInstance()->LoadTexture("Resources/terrain/terrain.png", "terrain");
	TextureManager::GetInstance()->LoadTexture("Resources/plane/plane.png", "plane");
	ModelManager::GetInstance()->LoadModelData("Resources/terrain", "terrain.obj");
	ModelManager::GetInstance()->LoadModelData("Resources/plane", "plane.gltf", true);
	ModelManager::GetInstance()->LoadModelData("Resources/plane", "plane.obj", true);
	AudioManager::GetInstance()->Load("Resources/audio/ohirusugi.mp3", "bgm");
}

TitleScene::~TitleScene () {
	//リソースを解放する
	TextureManager::GetInstance()->UnloadTexture("Resources/monsterBall.png");
	TextureManager::GetInstance()->UnloadTexture("Resources/uvChecker.png");
}

void TitleScene::Initialize (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) {
	camera_ = camera;
	input_ = inputManager;

	lightManager_ = std::make_unique<LightManager>(dxCommon);

	lightManager_->Initialize();

	sphere_ = std::make_unique<SphereMesh>(dxCommon, lightManager_.get());
	sphere_->Initialize({0.0f, 0.0f, 0.0f}, 3.0f);

	terrain_ = std::make_unique<Model>(dxCommon, lightManager_.get());
	terrain_->SetModelData("terrain.obj");
	terrain_->SetTexture("terrain");
	terrain_->Initialize();

	plane1_ = std::make_unique<Model>(dxCommon, lightManager_.get());
	plane1_->SetModelData("plane.gltf");
	plane1_->SetTexture("plane");
	plane1_->Initialize({ 3.0f, 3.0f, 1.0f }, {}, {-4.0f, 5.0f, 0.0f});

	plane2_ = std::make_unique<Model>(dxCommon, lightManager_.get());
	plane2_->SetModelData("plane.obj");
	plane2_->SetTexture("plane");
	plane2_->Initialize({ 3.0f, 3.0f, 1.0f }, {}, { 4.0f, 5.0f, 0.0f });

	bgm = AudioManager::GetInstance()->Play("bgm", BGM);
}

void TitleScene::Update () {
	if(input_->GetRawInput()->Trigger(VK_F1)) {
		nextScene_ = new PlayScene();
		SceneManager::GetInstance()->SetNextScene(nextScene_);
	}

	if(input_->GetRawInput()->Trigger(VK_SPACE)) {
		AudioManager::GetInstance()->SetVolume(bgm, 0.1f);
	}

	camera_->Update();

	sphere_->Update(&camera_->GetCameraData());
	sphere_->ImGui();

	terrain_->Update(&camera_->GetCameraData());
	terrain_->ImGui("terrain.obj");

	plane1_->Update(&camera_->GetCameraData());
	plane1_->ImGui("plane.gltf");

	plane2_->Update(&camera_->GetCameraData());
	plane2_->ImGui("plane.obj");

	lightManager_->Update();
	lightManager_->ImGui();
}

void TitleScene::Draw () {
	sphere_->Draw(TextureManager::GetInstance()->GetTextureHandle("uvChecker"));
	terrain_->Draw();
	plane1_->Draw();
	plane2_->Draw();
}