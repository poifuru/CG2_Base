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
#include "RenderSystem.h"
#include "InputManager.h"
#include "InputLayoutManager.h"
#include "BlendModeManager.h"

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

	// インプットレイアウトマネージャーの初期化
	InputLayoutManager::GetInstance()->Initialize();

	// ブレンドモードマネージャーの初期化
	BlendModeManager::GetInstance()->Initialize();

	// レンダーシステム生成
	renderSystem_ = std::make_unique<RenderSystem>();

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
	// ビューポートとシザー矩形の設定（描画の出力範囲を指定）
	D3D12_VIEWPORT viewport{};
	viewport.Width = static_cast<float>(winApi_->GetWindowWidth());
	viewport.Height = static_cast<float>(winApi_->GetWindowHeight());
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	D3D12_RECT scissorRect{};
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = winApi_->GetWindowWidth();
	scissorRect.bottom = winApi_->GetWindowHeight();

	ID3D12GraphicsCommandList* cmdList = cmdContext_->GetCommandList();
	cmdList->RSSetViewports(1, &viewport);
	cmdList->RSSetScissorRects(1, &scissorRect);

	// 溜まった描画コマンドを実行してクリアする
	renderSystem_->ExecuteCommands(
		device_->GetDevice(),
		cmdList,
		rootSigManager_->GetCommonRootSignature(),
		0, // cameraCBVAddress (今回は未使用なので0)
		*heapManager_,
		*psoManager_,
		*shaderManager_
	);
	renderSystem_->ClearCommands();

	// 描画終了をスワップチェーンに通知(PRESENTにバリア切り替え)
	swapChain_->EndRender(cmdContext_.get());

	cmdContext_->Execute();

	swapChain_->Present();

	cmdContext_->SignalAndWait();

	input_->EndFrame();
}

ID3D12Device* Engine::GetDevice() {
	return device_->GetDevice();;
}
