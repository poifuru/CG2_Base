#pragma once
#include "BaseScene.h"
#include "Model.h"
#include "LightManager.h"
#include "GameObject.h"

class LevelEditor;

class PlayScene : public BaseScene {
public:
	PlayScene();
	~PlayScene() override;

	void Initialize() override;
	void Update(CameraData* cameraData) override;
	void Draw(class RenderSystem* renderSystem) override;

private:
	// 全てのGameObject
	std::vector<std::unique_ptr<GameObject>> gameObjects_;

	// 選択中のGameObject
	GameObject* selectedObject_ = nullptr;

	std::unique_ptr<LightManager> lightManager_ = nullptr;

#ifdef USEIMGUI
	// エディタインスタンス
	std::unique_ptr<LevelEditor> levelEditor_ = nullptr;
	bool isDebugMode_ = true;
#endif
};
