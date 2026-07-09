#include "PCH.h"
#include "Engine.h"
#include "WindowsAPI.h"
#include "InputManager.h"
#include "FrameRateController.h"
#include "GraphicsDevice.h"
#include "DxcCompiler.h"
#include "CommandQueue.h"
#include "CommandList.h"
#include "SwapChain.h"
#include "DescriptorHeapManager.h"
#include "RootSignatureManager.h"
#include "ShaderManager.h"
#include "InputLayoutManager.h"
#include "BlendModeManager.h"
#include "PSOManager.h"
#include "RenderSystem.h"
#include "RenderTexture.h"
#include "CameraOrganizer.h"
#include "MainCameraComponent.h"
#include "LogManager.h"

Engine::Engine() = default;
Engine::~Engine() {
	WindowsAPI::GetInstance()->Finalize();
}

void Engine::Initialize() {
	WindowsAPI::GetInstance()->Initialize(1280, 720);

	LogManager::GetInstance()->Initialize();

	InputManager::GetInstance()->Initialize(WindowsAPI::GetInstance()->GetHwnd());

	frameRateController_ = std::make_unique<MyEngine::LowLevel::FrameRateController>();

	device_ = std::make_unique<MyEngine::LowLevel::GraphicsDevice>();
	device_->Initialize();

	dxcCompiler_ = std::make_unique<MyEngine::LowLevel::DxcCompiler>();
	dxcCompiler_->Initialize();

	cmdQueue_ = std::make_unique<MyEngine::LowLevel::CommandQueue>();
	cmdQueue_->Initialize(device_->GetDevice());

	cmdList_ = std::make_unique<MyEngine::LowLevel::CommandList>();
	cmdList_->Initialize(device_->GetDevice());

	swapChain_ = std::make_unique<MyEngine::LowLevel::SwapChain>();
	swapChain_->Initialize(
		device_->GetDxgiFactory(),
		cmdQueue_->GetCommandQueue(),
		WindowsAPI::GetInstance()->GetHwnd(),
		WindowsAPI::GetInstance()->GetWindowWidth(),
		WindowsAPI::GetInstance()->GetWindowHeight()
	);

	heapManager_ = std::make_unique<MyEngine::LowLevel::DescriptorHeapManager>();
	heapManager_->Initialize(device_->GetDevice(), 4096);

	rootSigManager_ = std::make_unique<RootSignatureManager>();
	rootSigManager_->Initialize(device_->GetDevice());

	shaderManager_ = std::make_unique<ShaderManager>();
	shaderManager_->Initialize(
		dxcCompiler_->GetDxcCompiler(),
		dxcCompiler_->GetDxcUtils(),
		dxcCompiler_->GetIncludeHandler()
	);

	inputLayoutManager_ = std::make_unique<InputLayoutManager>();
	inputLayoutManager_->Initialize();

	blendModeManager_ = std::make_unique<BlendModeManager>();
	blendModeManager_->Initialize();

	psoManager_ = std::make_unique<PSOManager>();

	renderSystem_ = std::make_unique<RenderSystem>();
	renderSystem_->Initialize(
		device_->GetDevice(),
		heapManager_.get(),
		psoManager_.get(),
		shaderManager_.get(),
		inputLayoutManager_.get(),
		blendModeManager_.get(),
		rootSigManager_->GetCommonRootSignature()
	);

	renderTexture_ = std::make_unique<RenderTexture>();
	renderTexture_->Initialize(device_->GetDevice(), heapManager_.get());
}

bool Engine::ProcessMessage() {
	return WindowsAPI::GetInstance()->ProcessMessage();
}

void Engine::BeginFrame() {
	frameRateController_->Update();
	InputManager::GetInstance()->Update();
	cmdList_->Reset();

#ifdef USEIMGUI
	// RenderTextureをレンダーターゲットに設定
	ID3D12GraphicsCommandList* cmdList = cmdList_->GetCommandList();
	cmdList_->TransitionBarrier(renderTexture_->GetResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = renderTexture_->GetDescriptorHandle();
	float clearColor[] = { 0.14f, 0.14f, 0.14f, 1.0f };
	cmdList_->ClearRenderTarget(rtvHandle, clearColor);

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = swapChain_->GetDsvHandle();
	cmdList_->ClearDepthBuffer(dsvHandle);

	cmdList_->SetRenderTargets(rtvHandle, &dsvHandle);
#else
	// 直接SwapChainのバックバッファに描画する
	float clearColor[] = { 0.14f, 0.14f, 0.14f, 1.0f };
	swapChain_->BeginRender(cmdList_.get(), clearColor);
#endif
}

void Engine::EndFrame() {

	swapChain_->EndRender(cmdList_.get());
	cmdQueue_->ExecuteCommandList(cmdList_->GetCommandList());
	swapChain_->Present();
	cmdQueue_->SignalAndWait();

	InputManager::GetInstance()->EndFrame();
}

void Engine::PreImGui() {
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

	ID3D12GraphicsCommandList* cmdList = cmdList_->GetCommandList();
	cmdList->RSSetViewports(1, &viewport);
	cmdList->RSSetScissorRects(1, &scissorRect);

	// カメラ座標を取得して RenderSystem に設定
	Vector3 cameraPos = CameraOrganizer::GetInstance()->GetCameraData().transform.translate;
	renderSystem_->SetCameraPosition(cameraPos);

	renderSystem_->ExecuteCommands(cmdList);
	renderSystem_->ClearCommands();

#ifdef USEIMGUI
	// RenderTextureへの描画が終わったのでSRVに遷移
	cmdList_->TransitionBarrier(renderTexture_->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	// SwapChainの準備 (ImGuiの描画先)
	float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f };
	swapChain_->BeginRender(cmdList_.get(), clearColor);
#endif
}

void Engine::ResetCommandList() {
	cmdList_->Reset();
}

void Engine::ExecuteCommandList() {
	cmdQueue_->ExecuteCommandList(cmdList_->GetCommandList());
	cmdQueue_->SignalAndWait();
}

ID3D12Device* Engine::GetDevice() {
	return device_->GetDevice();
}

ID3D12GraphicsCommandList* Engine::GetCommandList() {
	return cmdList_->GetCommandList();
}

ID3D12CommandQueue* Engine::GetCommandQueue() {
	return cmdQueue_->GetCommandQueue();
}

ShaderManager& Engine::GetShaderManager() {
	return *shaderManager_;
}