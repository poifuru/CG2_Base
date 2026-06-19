#include "RenderSystem.h"
#include "PSOManager.h"
#include "DescriptorHeapManager.h"
#include "ShaderManager.h"
#include <algorithm>
#include <cassert>

void RenderSystem::PushCommand(const RenderCommand& command) {
	commandQueue_.push_back(command);
}

void RenderSystem::ExecuteCommands(
	ID3D12Device* device,
	ID3D12GraphicsCommandList* cmdList,
	ID3D12RootSignature* commonRootSignature,
	D3D12_GPU_VIRTUAL_ADDRESS cameraCBVAddress,
	DescriptorHeapManager& heapManager,
	PSOManager& psoManager,
	const ShaderManager& shaderManager,
	const InputLayoutManager& inputLayoutManager,
	const BlendModeManager& blendModeManager
) {
	if (commandQueue_.empty()) return;

	// レイヤー順（不透明・半透明）にソート
	std::sort(commandQueue_.begin(), commandQueue_.end(), [](const RenderCommand& a, const RenderCommand& b) {
		return a.layer < b.layer;
	});

	// 共通ルートシグネチャをセット（フレームで1回固定）
	cmdList->SetGraphicsRootSignature(commonRootSignature);

	// 巨大ディスクリプタヒープをガツンとステージング（1回固定！）
	heapManager.SetGraphicsHeap(cmdList);

	// Slot 2 と Slot 3 に、巨大ヒープの「先頭のGPUハンドル」をセットして配列アクセス可能にする
	cmdList->SetGraphicsRootDescriptorTable(2, heapManager.GetGpuHandle(0));
	cmdList->SetGraphicsRootDescriptorTable(3, heapManager.GetGpuHandle(0));

	// Slot 0 にカメラバッファのアドレスをセット（1回固定！）
	if (cameraCBVAddress != 0) {
		cmdList->SetGraphicsRootConstantBufferView(0, cameraCBVAddress);
	}

	// トポロジーは三角形
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 重複するPSOをスキップする最適化のためのキャッシュ変数
	ID3D12PipelineState* lastPSO = nullptr;

	// コマンド実行ループ
	for (const auto& cmd : commandQueue_) {

		// 直前と同じパイプライン（PSO）なら、切り替え（SetPipelineState）を完全にスキップ！
		ID3D12PipelineState* currentPSO = psoManager.GetOrCreatePSO(
			device, cmd.psoDesc, commonRootSignature, shaderManager, inputLayoutManager, blendModeManager
		);
		if (currentPSO != lastPSO) {
			cmdList->SetPipelineState(currentPSO);
			lastPSO = currentPSO;
		}

		// メッシュ（三角形）のバインド
		cmdList->IASetVertexBuffers(0, 1, &cmd.vbView);
		cmdList->IASetIndexBuffer(&cmd.ibv);

		// Slot 1 に「マテリアル配列の何番目か」「テクスチャの何番目か」という
		// 32ビットの整数（数値）を、直接 Push Constants として2個流し込む
		uint32_t indices[2] = { cmd.materialIndex, cmd.textureIndex };
		cmdList->SetGraphicsRoot32BitConstants(1, 2, indices, 0);

		// ドローコール！
		cmdList->DrawIndexedInstanced(cmd.indexCount, 1, 0, 0, 0);
	}
}

void RenderSystem::ClearCommands() {
	commandQueue_.clear();
}