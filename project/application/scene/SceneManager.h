#pragma once
#include "BaseScene.h"
#include <memory>
#include "ModelManager.h"
#include "TextureManager.h"
#include "ModelFactory.h"

struct ID3D12Device;
struct ID3D12GraphicsCommandList;
class GraphicsDevice;
class DescriptorHeapManager;
class ShaderManager;
class InputManager;
struct CameraData;

class SceneManager {
public:
	SceneManager() = default;
	~SceneManager() = default;

	// 各マネージャーの初期化に必要なポインタ群を受け取って初期化
	void Initialize(
		ID3D12Device* device,
		GraphicsDevice* graphicsDevice,
		ID3D12GraphicsCommandList* cmdList,
		DescriptorHeapManager* heapManager,
		ShaderManager* shaderManager,
		InputManager* input
	);

	void Update(CameraData* cameraData);
	void Draw(class RenderSystem* renderSystem);
	void DrawUI();

	// シーン遷移用のテンプレート関数
	template <typename T>
	void ChangeScene() {
		auto nextScene = std::make_unique<T>();
		nextScene->SetContext(&context_);
		nextScene->Initialize();
		currentScene_ = std::move(nextScene);
	}

private:
	std::unique_ptr<BaseScene> currentScene_ = nullptr;

	// マネージャーの実体を SceneManager が所有する
	ModelManager modelManager_;
	TextureManager textureManager_;
	ModelFactory modelFactory_;

	// 各シーンへ配布する SceneContext の情報
	SceneContext context_;
};
