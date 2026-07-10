#include "PCH.h"
#include "Renderer.h"
#include "RootSignatureManager.h"
#include "ShaderManager.h"
#include "InputLayoutManager.h"
#include "BlendModeManager.h"
#include "PSOManager.h"
#include "RenderSystem.h"
#include "RenderTexture.h"
#include "WindowsAPI.h"
#include "CameraOrganizer.h"
#include "MainCameraComponent.h"
#include "Function.h"

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

	renderSystem_ = std::make_unique<MyEngine::Rendering::RenderSystem>();
	renderSystem_->Initialize(
		device,
		heapManager,
		psoManager_.get(),
		shaderManager_.get(),
		inputLayoutManager_.get(),
		blendModeManager_.get(),
		rootSigManager_->GetCommonRootSignature()
	);

	renderTexture_ = std::make_unique<MyEngine::Rendering::RenderTexture>();
	renderTexture_->Initialize(device, heapManager);
}

namespace {
	void SetupViewport(ID3D12GraphicsCommandList* cmdList) {
		D3D12_VIEWPORT viewport{};
		viewport.Width = static_cast<float>(WindowsAPI::GetInstance()->GetWindowWidth());
		viewport.Height = static_cast<float>(WindowsAPI::GetInstance()->GetWindowHeight());
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		cmdList->RSSetViewports(1, &viewport);
	}

	void SetupScissorRect(ID3D12GraphicsCommandList* cmdList) {
		D3D12_RECT scissorRect{};
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = WindowsAPI::GetInstance()->GetWindowWidth();
		scissorRect.bottom = WindowsAPI::GetInstance()->GetWindowHeight();

		cmdList->RSSetScissorRects(1, &scissorRect);
	}
}

void MyEngine::Rendering::Renderer::RenderScene(ID3D12GraphicsCommandList* cmdList) {
	SetupViewport(cmdList);
	SetupScissorRect(cmdList);

	// カメラ座標を取得して RenderSystem に設定
	Vector3 cameraPos = CameraOrganizer::GetInstance()->GetCameraData().transform.translate;
	renderSystem_->SetCameraPosition(cameraPos);

	renderSystem_->WriteCommandList(cmdList);
	renderSystem_->ClearCommands();

	// RenderTextureへの描画が終わったのでSRVに遷移
	MyEngine::Utility::TransitionBarrier(
		cmdList, 
		renderTexture_->GetResource(), 
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}