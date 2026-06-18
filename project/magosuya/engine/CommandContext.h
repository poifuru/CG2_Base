// =============================== //
// GPUへの命令の記録と送信、及び実行同期 //
// =============================== //

#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <cstdint>

class CommandContext {
public:
	CommandContext();
	~CommandContext();

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="device">デバイス</param>
	void Initialize(ID3D12Device* device);

	void Reset();

	void Execute();

	void SignalAndWait();

	// --- アクセッサ --- //
	ID3D12GraphicsCommandList* GetCommandList() const { return commandList_.Get(); }
	ID3D12CommandQueue* GetCommandQueue() const { return commandQueue_.Get(); }

	// --- コマンド記録のヘルパー --- //
	void TransitionBarrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter);
	void ClearRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle, const float color[4]);
	void SetRenderTargets(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle);

public:
	// コピー・移動禁止
	CommandContext(const CommandContext&) = delete;
	CommandContext& operator=(const CommandContext&) = delete;
	CommandContext(CommandContext&&) = delete;
	CommandContext& operator=(CommandContext&&) = delete;

private:
	// コマンド関連
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;

	// 同期（フェンス）用
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
	uint64_t fenceValue_ = 0;
	HANDLE fenceEvent_ = nullptr;
};