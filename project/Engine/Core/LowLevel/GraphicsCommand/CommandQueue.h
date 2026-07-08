#pragma once

// ＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝ //
// コマンドキューの保持、同期を行う //
// ＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝ //

namespace MyEngine::LowLevel {
	class CommandQueue {
	public:
		CommandQueue() = default;
		~CommandQueue();

		/// <summary>
		/// 初期化処理
		/// </summary>
		/// <param name="device"></param>
		void Initialize(Microsoft::WRL::ComPtr<ID3D12Device> device);

		/// <summary>
		/// コマンドを実行する
		/// </summary>
		/// <param name="cmdList"></param>
		void ExecuteCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& cmdList);


		void SignalAndWait();

	private:
		// コマンドキュー
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;

		// 同期（フェンス）用
		Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
		uint64_t fenceValue_ = 0;
		HANDLE fenceEvent_ = nullptr;
	};
}