#include "PCH.h"
#include "PlayScene.h"
#include "RenderSystem.h"
#include "CameraOrganizer.h"
#include "LevelEditor.h"
#include "ComponentType.h"
#include "InputManager.h"
#include "RawInput.h"

PlayScene::PlayScene() = default;
PlayScene::~PlayScene() = default;

void PlayScene::Initialize() {
	if (!context_) return;

#ifdef USEIMGUI
	levelEditor_ = std::make_unique<LevelEditor>();
	levelEditor_->Initialize(context_);
#endif

	// コンテキストにリストのポインタをセットする
	context_->gameObjects = &gameObjects_;

	// ライトマネージャーの初期化
	lightManager_ = std::make_unique<LightManager>();
	lightManager_->Initialize(context_->device);
}

void PlayScene::Update(CameraData* cameraData) {
	InputManager* input = InputManager::GetInstance();

	// Tabキーでプレイモード/デバッグモードを切り替える
	if (input->GetRawInput()->Trigger(VK_TAB)) {
		isDebugMode_ = !isDebugMode_;
		// シーン内のカメラを探して優先度（Priority）を切り替える
		for (auto& obj : gameObjects_) {
			// デバッグカメラの優先度設定
			if (auto* debugCam = obj->GetComponent<VirtualDebugCamera>()) {
				debugCam->SetPriority(isDebugMode_ ? 20 : 10);
			}
			// 追従カメラの優先度設定
			if (auto* followCam = obj->GetComponent<VirtualFollowCamera>()) {
				followCam->SetPriority(isDebugMode_ ? 10 : 20);
			}
		}
	}

	// 死亡フラグが立っているオブジェクトを削除
	CleanupObject();

	// 毎フレーム、全てのコンポーネントにデバッグ状態を通知する
	for (auto& obj : gameObjects_) {
		obj->SetIsDebugMode(isDebugMode_);
	}

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

void PlayScene::CleanupObject() {
	// デスフラグ（isDead_）が立っているオブジェクトをリストから削除する
	for (auto it = gameObjects_.begin(); it != gameObjects_.end();) {
		if ((*it)->IsDead()) {
			it = gameObjects_.erase(it); // メモリ解放＆削除
		} else {
			++it;
		}
	}
}
