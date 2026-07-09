#pragma once
#include "LeakChecker.h"

// 前方宣言
namespace MyEngine::LowLevel {
	class FrameRateController;
	class GraphicsDevice;
	class DxcCompiler;
	class CommandQueue;
	class CommandList;
	class SwapChain;
	class DescriptorHeapManager;
}
class RootSignatureManager;
class ShaderManager;
class InputLayoutManager;
class BlendModeManager;
class PSOManager;
class RenderSystem;
class RenderTexture;

class Engine {
public:
	Engine();
	~Engine();

	void Initialize();
	bool ProcessMessage();
	void BeginFrame();
	void EndFrame();
	void PreImGui();

	RenderSystem* GetRenderSystem() { return renderSystem_.get(); }

	// コマンド制御（オーバーライド）
	void ResetCommandList();
	void ExecuteCommandList();

	// 低レイヤーアクセッサ（オーバーライド）
	ID3D12Device* GetDevice();
	MyEngine::LowLevel::GraphicsDevice* GetGraphicsDevice() { return device_.get(); }
	MyEngine::LowLevel::DescriptorHeapManager* GetDescriptorHeapManager() { return heapManager_.get(); }
	ID3D12GraphicsCommandList* GetCommandList();
	ID3D12CommandQueue* GetCommandQueue();
	ShaderManager& GetShaderManager();
	RenderTexture* GetRenderTexture() { return renderTexture_.get(); }

private:
	LeakChecker leakCheck_{};

	// ---上から順に初期化、下から順に破棄--- //
	std::unique_ptr<MyEngine::LowLevel::FrameRateController> frameRateController_;
	std::unique_ptr<MyEngine::LowLevel::GraphicsDevice> device_;
	std::unique_ptr<MyEngine::LowLevel::DxcCompiler> dxcCompiler_;
	std::unique_ptr<MyEngine::LowLevel::CommandQueue> cmdQueue_;
	std::unique_ptr<MyEngine::LowLevel::CommandList> cmdList_;
	std::unique_ptr<MyEngine::LowLevel::SwapChain> swapChain_;
	std::unique_ptr<MyEngine::LowLevel::DescriptorHeapManager> heapManager_;

	std::unique_ptr<RootSignatureManager> rootSigManager_;
	std::unique_ptr<ShaderManager> shaderManager_;
	std::unique_ptr<InputLayoutManager> inputLayoutManager_;
	std::unique_ptr<BlendModeManager> blendModeManager_;
	std::unique_ptr<PSOManager> psoManager_;

	std::unique_ptr<RenderSystem> renderSystem_;
	std::unique_ptr<RenderTexture> renderTexture_;
};