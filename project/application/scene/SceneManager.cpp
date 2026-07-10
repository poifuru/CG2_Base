#include "PCH.h"
#include "SceneManager.h"
#include "RenderSystem.h"
#include "GraphicsDevice.h"

void SceneManager::Initialize(
	MyEngine::LowLevel::GraphicsDevice* graphicsDevice,
	ID3D12GraphicsCommandList* cmdList,
	MyEngine::LowLevel::DescriptorHeapManager* heapManager,
	ShaderManager* shaderManager
) {
	// マネージャー群を初期化
	textureManager_.Initialize(graphicsDevice->GetDevice(), cmdList, heapManager);
	modelManager_.Initialize(graphicsDevice->GetDevice(), &textureManager_);
	modelFactory_.Initialize(graphicsDevice, heapManager, &modelManager_, &textureManager_, shaderManager);

	// シーン配布用のコンテキストを組み立てる
	context_.graphicsDevice = graphicsDevice;
	context_.heapManager = heapManager;
	context_.textureManager = &textureManager_;
	context_.modelFactory = &modelFactory_;
	context_.shaderManager = shaderManager;
	context_.modelManager = &modelManager_;
}

void SceneManager::Update(CameraData* cameraData) {
	if (currentScene_) {
		currentScene_->Update(cameraData);
	}
}

void SceneManager::Draw(MyEngine::Rendering::RenderSystem* renderSystem) {
	if (currentScene_) {
		currentScene_->Draw(renderSystem);
	}
}

void SceneManager::DrawUI() {
	if (currentScene_) {
		currentScene_->DrawUI();
	}
}