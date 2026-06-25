#include "PCH.h"
#include "PlayScene.h"
#include "RenderSystem.h"
#include "CameraOrganizer.h"
#include "LevelEditor.h"
#include "ComponentType.h"

PlayScene::PlayScene() = default;
PlayScene::~PlayScene() = default;

void PlayScene::Initialize() {
	if (!context_) return;

	// ライトマネージャーの初期化
	lightManager_ = std::make_unique<LightManager>();
	lightManager_->Initialize(context_->device);

	// シーン初期化時にターゲットを紐づけ
	for (auto& obj : gameObjects_) {
		if (auto* followCam = obj->GetComponent<VirtualFollowCamera>()) {
			followCam->ResolveTarget(gameObjects_);
		}
	}

#ifdef USEIMGUI
	levelEditor_ = std::make_unique<LevelEditor>();
	levelEditor_->Initialize(context_);
#endif
}

void PlayScene::Update(CameraData* cameraData) {
	// 全オブジェクトの更新
	for (auto& obj : gameObjects_) {
		obj->Update();
	}

	// カメラの更新
	CameraOrganizer::GetInstance()->Update();

	// ライトの更新
	if (lightManager_) {
		lightManager_->ClearLights();

		// 全オブジェクトからLightComponentを探して登録
		for(auto& obj : gameObjects_) {
			if(auto* light = obj->GetComponent<LightComponent>()) {
				lightManager_->Register(light);
			}
		}

		lightManager_->Update();
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