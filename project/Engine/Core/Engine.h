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

		/// <summary>
		/// 初期化
		/// </summary>
		void Initialize();

		/// <summary>
		/// ウィンドウからのメッセージを処理する
		/// </summary>
		/// <returns></returns>
		bool ProcessMessage();

		/// <summary>
		/// 描画前処理
		/// </summary>
		/// <param name="renderTexResource"></param>
		/// <param name="renderTexDescriptorHandle"></param>
		void BeginFrame(
			ID3D12Resource* renderTexResource,
			D3D12_CPU_DESCRIPTOR_HANDLE renderTexDescriptorHandle
		);

		/// <summary>
		/// 描画後処理
		/// </summary>
		void EndFrame();

		/// <summary>
		/// ImGui描画前処理
		/// </summary>
		void BeginSwapChainRender();

		// コマンド制御
		void ResetCommandList();
		void ExecuteCommandList();

		// 低レイヤーアクセッサ
		ID3D12Device* GetDevice();
		MyEngine::LowLevel::GraphicsDevice* GetGraphicsDevice();
		ID3D12GraphicsCommandList* GetCommandList();
		ID3D12CommandQueue* GetCommandQueue();
		IDxcUtils* GetDxcUtils();
		IDxcCompiler3* GetDxcCompiler();
		IDxcIncludeHandler* GetIncludeHandler();

		MyEngine::LowLevel::DescriptorHeapManager* GetDescriptorHeapManager() { return heapManager_.get(); }

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