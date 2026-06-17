#pragma once
#include "IEngine.h"
#include <memory>

// 前方宣言でアプリケーション側に詳細な型を隠す
class WindowsAPI;
class FrameRateController;
class GraphicsDevice;
class CommandContext;
class SwapChain;
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
	// 上から順に初期化、下から順に破棄する
	std::unique_ptr<WindowsAPI> winApi_;
	std::unique_ptr<FrameRateController> frameRateController_;
	std::unique_ptr<GraphicsDevice> device_;
	std::unique_ptr<CommandContext> cmdContext_;
	std::unique_ptr<SwapChain> swapChain_;
	std::unique_ptr<InputManager> input_;
};