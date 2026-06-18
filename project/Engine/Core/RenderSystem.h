#pragma once
#include <vector>
#include <d3d12.h>
#include "RenderCommand.h"

class PSOManager;
class DescriptorHeapManager;
class ShaderManager;

class RenderSystem {
public:
	RenderSystem() = default;
	~RenderSystem() = default;

	// コマンドの積み込み
	void PushCommand(const RenderCommand& command);

	// 必要なマネージャーやコマンドリスト、共通シグネチャ、デバイスをすべて引数で貰って駆動する
	void ExecuteCommands(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		ID3D12RootSignature* commonRootSignature,
		D3D12_GPU_VIRTUAL_ADDRESS cameraCBVAddress, // Slot 0 用のカメラアドレス
		DescriptorHeapManager& heapManager,         // Slot 2 用の巨大ヒープ
		PSOManager& psoManager,
		const ShaderManager& shaderManager
	);

	void ClearCommands();

public:
	// コピー・移動禁止
	RenderSystem(const RenderSystem&) = delete;
	RenderSystem& operator=(const RenderSystem&) = delete;
	RenderSystem(RenderSystem&&) = delete;
	RenderSystem& operator=(RenderSystem&&) = delete;

private:
	std::vector<RenderCommand> commandQueue_;
};