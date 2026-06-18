// =================================== //
// 画面をフリップさせるためのスワップチェイン //
// =================================== //

#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <array>
#include <cstdint>

class CommandContext;

class SwapChain {
public:
	// トリプルバッファリングのためのバッファ数定数
	static constexpr uint32_t kBufferCount = 3;

	SwapChain();
	~SwapChain() = default;

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="dxgiFactory">DXGIファクトリー</param>
	/// <param name="cmdQueue">コマンドキュー</param>
	/// <param name="hwnd">ウィンドウハンドル</param>
	/// <param name="width">ウィンドウサイズ(横)</param>
	/// <param name="height">ウィンドウサイズ(縦)</param>
	void Initialize(
		IDXGIFactory7* dxgiFactory,
		ID3D12CommandQueue* cmdQueue,
		HWND hwnd,
		int32_t width,
		int32_t height
	);

	/// <summary>
	/// 画面をフリップさせる
	/// </summary>
	void Present();

	/// <summary>
	/// 描画開始処理（バリア遷移・クリア・ターゲット設定をまとめて行う）
	/// </summary>
	void BeginRender(CommandContext* cmdContext, const float clearColor[4]);

	/// <summary>
	/// 描画終了処理（バリアをPRESENTに戻す）
	/// </summary>
	void EndRender(CommandContext* cmdContext);

	// --- アクセッサ --- //
	uint32_t GetCurrentBackBufferIndex() const { return swapChain_->GetCurrentBackBufferIndex(); }
	ID3D12Resource* GetBackBufferResource(uint32_t index) const { return swapChainResources_[index].Get(); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetRtvHandle(uint32_t index) const { return rtvHandles_[index]; }

public:
	// コピー・移動禁止
	SwapChain(const SwapChain&) = delete;
	SwapChain& operator=(const SwapChain&) = delete;
	SwapChain(SwapChain&&) = delete;
	SwapChain& operator=(SwapChain&&) = delete;

private:
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;

	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, kBufferCount> swapChainResources_;
	std::array<D3D12_CPU_DESCRIPTOR_HANDLE, kBufferCount> rtvHandles_;

	// RTV専用のディスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_;
};