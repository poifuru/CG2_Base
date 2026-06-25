#pragma once
#include "GameObject.h"
#include "struct.h"

struct SceneContext;
struct CameraData;

class LevelEditor {
public:
	LevelEditor() = default;
	~LevelEditor() = default;
	void Initialize(SceneContext* context);

	// PlaySceneのGameObjectリストと、選択中のオブジェクトのポインタを渡して更新する
	void Update(
		std::vector<std::unique_ptr<GameObject>>& gameObjects, 
		GameObject*& selectedObject, 
		CameraData* cameraData
	);
private:
	// フォルダツリー描画などのプライベート関数もここに引っ越す
	void DrawDirectoryTree(const std::filesystem::path& path);
	void SaveScene(const std::vector<std::unique_ptr<GameObject>>& gameObjects);
	void LoadScene(std::vector<std::unique_ptr<GameObject>>& gameObjects, GameObject*& selectedObject);
private:
	SceneContext* context_ = nullptr;
	std::filesystem::path currentDirectory_ = "Resources";
};