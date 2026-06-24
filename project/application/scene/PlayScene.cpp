#include "PCH.h"
#include "PlayScene.h"
#include "RenderSystem.h"
#include "CameraOrganizer.h"
#include "LevelEditor.h"

PlayScene::PlayScene() = default;
PlayScene::~PlayScene() = default;

void PlayScene::Initialize() {
	if (!context_) return;

	// ライトマネージャーの初期化
	lightManager_ = std::make_unique<LightManager>();
	lightManager_->Initialize(context_->device);

	// 初期ライトとして DirectionalLight を 1 つ追加して設定
	lightManager_->AddLight(LightType::DIRECTIONALLIGHT);
	lightManager_->SetDirectionalLightDir(0, { 0.5f, -1.0f, 0.5f }); // 斜め下
	lightManager_->SetDirectionalLightColor(0, { 1.0f, 1.0f, 1.0f, 1.0f }); // 白色
	lightManager_->SetDirectionalLightIntensity(0, 1.0f); // 輝度 1.0

#ifdef USEIMGUI
	levelEditor_ = std::make_unique<LevelEditor>();
	levelEditor_->Initialize(context_);
#endif
}

void PlayScene::Update(CameraData* cameraData) {
	// カメラの更新
	CameraOrganizer::GetInstance()->Update();

	// ライトの更新
	if (lightManager_) {
		lightManager_->Update();
	}

	for (auto& obj : gameObjects_) {
		obj->Update();
	}

#ifdef USEIMGUI
	// エディタの更新処理に丸投げ！
	levelEditor_->Update(gameObjects_, selectedObject_, cameraData);
#endif
}

void PlayScene::Draw(RenderSystem* renderSystem) {
	// ライトマネージャーを RenderSystem に登録
	if (lightManager_ && renderSystem) {
		renderSystem->SetLightManager(lightManager_.get());
	}

	for (auto& obj : gameObjects_) {
		obj->Draw(renderSystem);
	}
}