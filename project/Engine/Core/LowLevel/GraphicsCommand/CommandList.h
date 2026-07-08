#pragma once

// ＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝ //
// コマンドリストの作成、記録を行う //
// ＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝ //

namespace MyEngine::LowLevel {
	class CommandList {
	public:
		CommandList() = default;
		~CommandList() = default;

		/// <summary>
		/// 初期化処理
		/// </summary>
		/// <param name="device"></param>
		void Initialize(Microsoft::WRL::ComPtr<ID3D12Device> device);

		/// <summary>
		/// アロケータとリストのリセット
		/// </summary>
		void Reset();

	private:
		// コマンドアロケータ
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
		// コマンドリスト
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
	};
}