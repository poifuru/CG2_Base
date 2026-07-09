#include "PCH.h"
#include "Renderer.h"
#include "RootSignatureManager.h"
#include "ShaderManager.h"
#include "InputLayoutManager.h"
#include "BlendModeManager.h"
#include "PSOManager.h"
#include "RenderSystem.h"
#include "RenderTexture.h"

void MyEngine::Rendering::Renderer::Initialize(
	ID3D12Device* device,
	IDxcUtils* dxcUtils,
	IDxcCompiler3* dxcCompiler,
	IDxcIncludeHandler* includeHandler,
	MyEngine::LowLevel::DescriptorHeapManager* heapManager
) {
	rootSigManager_ = std::make_unique<RootSignatureManager>();
	rootSigManager_->Initialize(device);

	shaderManager_ = std::make_unique<ShaderManager>();
	shaderManager_->Initialize(
		dxcUtils,
		dxcCompiler,
		includeHandler
	);

	inputLayoutManager_ = std::make_unique<InputLayoutManager>();
	inputLayoutManager_->Initialize();

	blendModeManager_ = std::make_unique<BlendModeManager>();
	blendModeManager_->Initialize();

	psoManager_ = std::make_unique<PSOManager>();

	renderSystem_ = std::make_unique<RenderSystem>();
	renderSystem_->Initialize(
		device,
		heapManager,
		psoManager_.get(),
		shaderManager_.get(),
		inputLayoutManager_.get(),
		blendModeManager_.get(),
		rootSigManager_->GetCommonRootSignature()
	);

	renderTexture_ = std::make_unique<RenderTexture>();
	renderTexture_->Initialize(device, heapManager);
}