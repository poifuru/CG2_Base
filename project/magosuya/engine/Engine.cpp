#include "Engine.h"
#include "WindowsAPI.h"
#include "GraphicsDevice.h"
#include "CommandContext.h"
#include "SwapChain.h"
#include "FrameRateController.h"
#include "InputManager.h"

Engine::Engine() = default;
Engine::~Engine() = default;

void Engine::Initialize() {
	// ウィンドウ生成
	winApi_ = std::make_unique<WindowsAPI>();
	winApi_->Initialize(1280, 720);

	// フレームレートコントーラー生成
	frameRateController_ = std::make_unique<FrameRateController>();

	// グラフィックスの基盤を作成
	device_ = std::make_unique<GraphicsDevice>();
	device_->Initialize();

	// コマンドコンテキストを作成
	cmdContext_ = std::make_unique<CommandContext>();
	cmdContext_->Initialize(device_->GetDevice());

	// SwapChainの生成
	swapChain_ = std::make_unique<SwapChain>();
	swapChain_->Initialize(
		device_->GetDxgiFactory(),
		cmdContext_->GetCommandQueue(),
		winApi_->GetHwnd(),
		winApi_->GetWindowWidth(),
		winApi_->GetWindowHeight()
	);

	// インプットマネージャーの生成
	input_ = std::make_unique<InputManager>();
	input_->Initialize(winApi_->GetHwnd());
}

bool Engine::ProcessMessage() {
	return winApi_->ProcessMessage();
}

void Engine::BeginFrame() {
	// フレームの最初で正確な経過時間を計測
	frameRateController_->Update();

	input_->Update();

	cmdContext_->Reset();
}

void Engine::EndFrame() {
	cmdContext_->Execute();

	swapChain_->Present();

	cmdContext_->SignalAndWait();

	input_->EndFrame();
}
