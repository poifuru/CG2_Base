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

namespace MyEngine::LowLevel {
	class Engine {
	public:
		Engine();
		~Engine();

		void Initialize();
		bool ProcessMessage();
		void BeginFrame(
			ID3D12Resource* renderTexResource,
			D3D12_CPU_DESCRIPTOR_HANDLE renderTexDescriptorHandle
		);
		void EndFrame();
		void PreImGui();

		// コマンド制御
		void ResetCommandList();
		void ExecuteCommandList();

		// 低レイヤーアクセッサ
		ID3D12Device* GetDevice();
		MyEngine::LowLevel::GraphicsDevice* GetGraphicsDevice() { return device_.get(); }
		MyEngine::LowLevel::DescriptorHeapManager* GetDescriptorHeapManager() { return heapManager_.get(); }
		ID3D12GraphicsCommandList* GetCommandList();
		ID3D12CommandQueue* GetCommandQueue();

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
	};
}