#pragma once
#include "BaseScene.h"
#include <vector>
#include <memory>
#include "Model.h"
#include "LightManager.h"
#include "GameObject.h"
#include <filesystem>

class PlayScene : public BaseScene {
public:
	PlayScene() = default;
	~PlayScene() override = default;

	void Initialize() override;
	void Update(CameraData* cameraData) override;
	void Draw(class RenderSystem* renderSystem) override;

private:
	// ディレクトリツリーを再帰的に描画するヘルパー関数
	void DrawDirectoryTree(const std::filesystem::path& path);

private:
	// 全てのGameObject
	std::vector<std::unique_ptr<GameObject>> gameObjects_;

	// アセットブラウザのカレントディレクトリ
	std::filesystem::path currentDirectory_ = "Resources";

	// 選択中のGameObject
	GameObject* selectedObject_ = nullptr;

	std::unique_ptr<LightManager> lightManager_ = nullptr;
};
