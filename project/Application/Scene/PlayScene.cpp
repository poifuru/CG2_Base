#include "PlayScene.h"
#include "RenderSystem.h"
#include "TextureManager.h"
#include "ModelFactory.h"
#include "ModelManager.h"
#include "CameraOrganizer.h"
#include "LogManager.h"
#include <format>

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

	// 3Dモデルをロード（内部でマテリアル・テクスチャのロードも行われます）
	uint32_t modelIndex = context_->modelManager->LoadModelData("Resources/teapot/teapot.obj");

	// モデルを生成（第2引数を 0 にすることでモデル本来のマテリアルテクスチャを使用）
	model_ = context_->modelFactory->CreateModel(modelIndex);
}

void PlayScene::Update(CameraData* cameraData) {
	// カメラの更新
	CameraOrganizer::GetInstance()->Update();

	// ライトの更新
	if (lightManager_) {
		lightManager_->Update();
#ifdef USEIMGUI
		//lightManager_->ImGui(); // ImGuiの表示
#endif
	}

	if (model_) {
		model_->Update(cameraData);
	}
}

void PlayScene::Draw(RenderSystem* renderSystem) {
	// ライトマネージャーを RenderSystem に登録
	if (lightManager_ && renderSystem) {
		renderSystem->SetLightManager(lightManager_.get());
	}

	if (model_) {
		model_->Draw(renderSystem);
	}
}