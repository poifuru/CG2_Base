#pragma once

// ＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝ //
// コマンドキュー、アロケータ、リスト、フェンスの生成のみを行う //
// ＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝ //

namespace MyEngine::LowLevel {
	class GraphicsCommandCreator {
	public:
		/// <summary>
		/// コマンドキューを作成
		/// </summary>
		/// <param name="device"></param>
		/// <param name="commandQueue"></param>
		static void CreateCommandQueue(
			Microsoft::WRL::ComPtr<ID3D12Device> device,
			Microsoft::WRL::ComPtr<ID3D12CommandQueue>& commandQueue
		);

		/// <summary>
		/// コマンドアロケータを作成
		/// </summary>
		/// <param name="device"></param>
		/// <param name="allocator"></param>
		static void CreateCommandAllocator(
			Microsoft::WRL::ComPtr<ID3D12Device> device,
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& commandAllocator
		);

		/// <summary>
		/// コマンドリストを作成
		/// </summary>
		/// <param name="device"></param>
		/// <param name="commandAllocator"></param>
		/// <param name="commandList"></param>
		static void CreateCommandList(
			Microsoft::WRL::ComPtr<ID3D12Device> device,
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& commandAllocator,
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList
		);

		/// <summary>
		/// フェンスを作成
		/// </summary>
		/// <param name="device"></param>
		/// <param name="fence"></param>
		static void CreateFence(
			Microsoft::WRL::ComPtr<ID3D12Device> device,
			Microsoft::WRL::ComPtr<ID3D12Fence>& fence,
			uint64_t& fenceValue,
			HANDLE& fenceEvent
		);
	};
}