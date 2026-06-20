#include "SceneManager.h"
#include "RenderSystem.h"

void SceneManager::Initialize(
	ID3D12Device* device,
	GraphicsDevice* graphicsDevice,
	ID3D12GraphicsCommandList* cmdList,
	DescriptorHeapManager* heapManager,
	ShaderManager* shaderManager
) {
	// マネージャー群を初期化
	textureManager_.Initialize(device, cmdList, heapManager);
	modelManager_.Initialize(device, &textureManager_);
	modelFactory_.Initialize(graphicsDevice, heapManager, &modelManager_, &textureManager_, shaderManager);

	// シーン配布用のコンテキストを組み立てる
	context_.textureManager = &textureManager_;
	context_.modelFactory = &modelFactory_;
	context_.shaderManager = shaderManager;
	context_.modelManager = &modelManager_;
	context_.device = device;
	context_.cmdList = cmdList;
	context_.heapManager = heapManager;
}

void SceneManager::Update(CameraData* cameraData) {
	if (currentScene_) {
		currentScene_->Update(cameraData);
	}
}

void SceneManager::Draw(RenderSystem* renderSystem) {
	if (currentScene_) {
		currentScene_->Draw(renderSystem);
	}
}

void SceneManager::DrawUI() {
	if (currentScene_) {
		currentScene_->DrawUI();
	}
}