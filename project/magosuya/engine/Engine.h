#pragma once
#include <memory>
#include "IEngine.h"
#include "LeakChecker.h"

// 前方宣言でアプリケーション側に詳細な型を隠す
class WindowsAPI;
class FrameRateController;
class GraphicsDevice;
class CommandContext;
class SwapChain;
class DescriptorHeapManager;
class RootSignatureManager;
class ShaderManager;
class PSOManager;
class InputManager;

class Engine : public IEngine {
public:
	Engine();
	~Engine() override;

	void Initialize() override;
	bool ProcessMessage() override;
	void BeginFrame() override;
	void EndFrame() override;

private:
	// リークチェッカー
	LeakChecker leakCheck_{};

	// ---上から順に初期化、下から順に破棄する --- //
	//  *** 低レイヤー層 *** //
	std::unique_ptr<WindowsAPI> winApi_;
	std::unique_ptr<FrameRateController> frameRateController_;
	std::unique_ptr<GraphicsDevice> device_;
	std::unique_ptr<CommandContext> cmdContext_;
	std::unique_ptr<SwapChain> swapChain_;

	// *** 中間レイヤー層 *** //
	std::unique_ptr<DescriptorHeapManager> heapManager_;

	// *** 描画パイプライン*** //
	std::unique_ptr<RootSignatureManager> rootSigManager_;
	std::unique_ptr<ShaderManager> shaderManager_;
	std::unique_ptr<PSOManager> psoManager_;
	std::unique_ptr<InputManager> input_;
};