#pragma once
#include "BaseScene.h"
#include <vector>
#include <memory>
#include "Model.h"
#include "LightManager.h"
#include "GameObject.h"

class PlayScene : public BaseScene {
public:
	PlayScene() = default;
	~PlayScene() override = default;

	void Initialize() override;
	void Update(CameraData* cameraData) override;
	void Draw(class RenderSystem* renderSystem) override;

private:
	// アセットをスキャンしてリストを更新する関数
	void RefreshAssetList();

private:
	// 全てのGameObject
	std::vector<std::unique_ptr<GameObject>> gameObjects_;
	// アセットフォルダ内のモデルファイルのパス一覧
	std::vector<std::string> modelFiles_;

	// 選択中のGameObject
	GameObject* selectedObject_ = nullptr;

	std::unique_ptr<LightManager> lightManager_ = nullptr;
};
