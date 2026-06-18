#include "Engine.h"
#include "WindowsAPI.h"
#include "FrameRateController.h"
#include "GraphicsDevice.h"
#include "CommandContext.h"
#include "SwapChain.h"
#include "DescriptorHeapManager.h"
#include "RootSignatureManager.h"
#include "ShaderManager.h"
#include "PSOManager.h"
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

	// ディスクリプタヒープマネージャーの生成
	heapManager_ = std::make_unique<DescriptorHeapManager>();
	heapManager_->Initialize(device_->GetDevice(), 4096);

	// ルートシグネチャマネージャーの生成
	rootSigManager_ = std::make_unique<RootSignatureManager>();
	rootSigManager_->Initialize(device_->GetDevice());

	// シェーダーマネージャーの生成
	shaderManager_ = std::make_unique<ShaderManager>();
	shaderManager_->Initialize(
		device_->GetDxcCompiler(),
		device_->GetDxcUtils(),
		device_->GetIncludeHandler()
	);

	// PSOマネージャーの生成
	psoManager_ = std::make_unique<PSOManager>();

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

	// 画面をクリアカラー（青っぽい色）でクリアして描画準備
	float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f };
	swapChain_->BeginRender(cmdContext_.get(), clearColor);
}

void Engine::EndFrame() {
	// 描画終了をスワップチェーンに通知(PRESENTにバリア切り替え)
	swapChain_->EndRender(cmdContext_.get());

	cmdContext_->Execute();

	swapChain_->Present();

	cmdContext_->SignalAndWait();

	input_->EndFrame();
}
