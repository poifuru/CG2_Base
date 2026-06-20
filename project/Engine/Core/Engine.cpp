#include "Engine.h"
#include "WindowsAPI.h"
#include "InputManager.h"
#include "FrameRateController.h"
#include "GraphicsDevice.h"
#include "CommandContext.h"
#include "SwapChain.h"
#include "DescriptorHeapManager.h"
#include "RootSignatureManager.h"
#include "ShaderManager.h"
#include "InputLayoutManager.h"
#include "BlendModeManager.h"
#include "PSOManager.h"
#include "RenderSystem.h"
#include "CameraOrganizer.h"
#include "LogManager.h"

Engine::Engine() = default;
Engine::~Engine() {
	WindowsAPI::GetInstance()->Finalize();
}

void Engine::Initialize() {
	WindowsAPI::GetInstance()->Initialize(1280, 720);

	LogManager::GetInstance()->Initialize();

	InputManager::GetInstance()->Initialize(WindowsAPI::GetInstance()->GetHwnd());

	frameRateController_ = std::make_unique<FrameRateController>();

	device_ = std::make_unique<GraphicsDevice>();
	device_->Initialize();

	cmdContext_ = std::make_unique<CommandContext>();
	cmdContext_->Initialize(device_->GetDevice());

	swapChain_ = std::make_unique<SwapChain>();
	swapChain_->Initialize(
		device_->GetDxgiFactory(),
		cmdContext_->GetCommandQueue(),
		WindowsAPI::GetInstance()->GetHwnd(),
		WindowsAPI::GetInstance()->GetWindowWidth(),
		WindowsAPI::GetInstance()->GetWindowHeight()
	);

	heapManager_ = std::make_unique<DescriptorHeapManager>();
	heapManager_->Initialize(device_->GetDevice(), 4096);

	rootSigManager_ = std::make_unique<RootSignatureManager>();
	rootSigManager_->Initialize(device_->GetDevice());

	shaderManager_ = std::make_unique<ShaderManager>();
	shaderManager_->Initialize(
		device_->GetDxcCompiler(),
		device_->GetDxcUtils(),
		device_->GetIncludeHandler()
	);

	inputLayoutManager_ = std::make_unique<InputLayoutManager>();
	inputLayoutManager_->Initialize();

	blendModeManager_ = std::make_unique<BlendModeManager>();
	blendModeManager_->Initialize();

	psoManager_ = std::make_unique<PSOManager>();

	renderSystem_ = std::make_unique<RenderSystem>();
	renderSystem_->Initialize(device_->GetDevice());
}

bool Engine::ProcessMessage() {
	return WindowsAPI::GetInstance()->ProcessMessage();
}

void Engine::BeginFrame() {
	frameRateController_->Update();
	InputManager::GetInstance()->Update();
	cmdContext_->Reset();

	float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f };
	swapChain_->BeginRender(cmdContext_.get(), clearColor);
}

void Engine::EndFrame() {
	D3D12_VIEWPORT viewport{};
	viewport.Width = static_cast<float>(WindowsAPI::GetInstance()->GetWindowWidth());
	viewport.Height = static_cast<float>(WindowsAPI::GetInstance()->GetWindowHeight());
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	D3D12_RECT scissorRect{};
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = WindowsAPI::GetInstance()->GetWindowWidth();
	scissorRect.bottom = WindowsAPI::GetInstance()->GetWindowHeight();

	ID3D12GraphicsCommandList* cmdList = cmdContext_->GetCommandList();
	cmdList->RSSetViewports(1, &viewport);
	cmdList->RSSetScissorRects(1, &scissorRect);

	// カメラ座標を取得して RenderSystem に設定
	Vector3 cameraPos = CameraOrganizer::GetInstance()->GetCameraData().transform.translate;
	renderSystem_->SetCameraPosition(cameraPos);

	renderSystem_->ExecuteCommands(
		device_->GetDevice(),
		cmdList,
		rootSigManager_->GetCommonRootSignature(),
		*heapManager_,
		*psoManager_,
		*shaderManager_,
		*inputLayoutManager_,
		*blendModeManager_
	);
	renderSystem_->ClearCommands();

	swapChain_->EndRender(cmdContext_.get());
	cmdContext_->Execute();
	swapChain_->Present();
	cmdContext_->SignalAndWait();

	InputManager::GetInstance()->EndFrame();
}

void Engine::ResetCommandList() {
	cmdContext_->Reset();
}

void Engine::ExecuteCommandList() {
	cmdContext_->Execute();
	cmdContext_->SignalAndWait();
}

ID3D12Device* Engine::GetDevice() {
	return device_->GetDevice();
}

ID3D12GraphicsCommandList* Engine::GetCommandList() {
	return cmdContext_->GetCommandList();
}

ShaderManager& Engine::GetShaderManager() {
	return *shaderManager_;
}
