#pragma once
#include <memory>
#include "IEngine.h"
#include "LeakChecker.h"

// 前方宣言
struct ID3D12Device;
struct ID3D12GraphicsCommandList;
class WindowsAPI;
class InputManager;
class FrameRateController;
class GraphicsDevice;
class CommandContext;
class SwapChain;
class DescriptorHeapManager;
class RootSignatureManager;
class ShaderManager;
class InputLayoutManager;
class BlendModeManager;
class PSOManager;
class RenderSystem;

class Engine : public IEngine {
public:
	Engine();
	~Engine() override;

	void Initialize() override;
	bool ProcessMessage() override;
	void BeginFrame() override;
	void EndFrame() override;

	RenderSystem* GetRenderSystem() override { return renderSystem_.get(); }

	// アセットロード用コマンド制御
	void ResetCommandList();
	void ExecuteCommandList();

	// 低レイヤーアクセッサ
	ID3D12Device* GetDevice();
	GraphicsDevice* GetGraphicsDevice() { return device_.get(); }
	DescriptorHeapManager* GetDescriptorHeapManager() { return heapManager_.get(); }
	ID3D12GraphicsCommandList* GetCommandList();
	InputManager* GetInputManager() { return input_.get(); }
	ShaderManager& GetShaderManager();

private:
	LeakChecker leakCheck_{};

	// ---上から順に初期化、下から順に破棄--- //
	std::unique_ptr<WindowsAPI> winApi_;
	std::unique_ptr<InputManager> input_;
	std::unique_ptr<FrameRateController> frameRateController_;
	std::unique_ptr<GraphicsDevice> device_;
	std::unique_ptr<CommandContext> cmdContext_;
	std::unique_ptr<SwapChain> swapChain_;
	std::unique_ptr<DescriptorHeapManager> heapManager_;

	std::unique_ptr<RootSignatureManager> rootSigManager_;
	std::unique_ptr<ShaderManager> shaderManager_;
	std::unique_ptr<InputLayoutManager> inputLayoutManager_;
	std::unique_ptr<BlendModeManager> blendModeManager_;
	std::unique_ptr<PSOManager> psoManager_;

	std::unique_ptr<RenderSystem> renderSystem_;
};