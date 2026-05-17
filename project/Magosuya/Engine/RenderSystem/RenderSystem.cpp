#include "RenderSystem.h"
#include "SRVManager.h"
#include "TextureManager.h"
#include "DxCommon.h"
#include "LightManager.h"
#include <algorithm>

void RenderSystem::Initialize(DxCommon* dxCommon, LightManager* lightManager) {
	dxCommon_ = dxCommon;
	commandList_ = dxCommon->GetCommandList();
	lightManager_ = lightManager;
}

void RenderSystem::PushCommand(const RenderCommand& command) {
	commandQueue_.push_back(command);
}

void RenderSystem::ExecuteCommands(D3D12_GPU_VIRTUAL_ADDRESS cameraCBVAddress) {
	if(commandQueue_.empty()) return;

	// コマンドをソート
	std::sort(commandQueue_.begin(), commandQueue_.end(), [](const RenderCommand& a, const RenderCommand& b) {
		return a.layer < b.layer;
	});

	uint32_t currentRootSig = 0;
	// 重複するPSOをスキップする最適化を入れたい

	for(const auto& cmd : commandQueue_) {
		// ルートシグネチャ、PSOの設定
		RootSignatureManager::GetInstance()->SetRootSignature(cmd.rootSignatureID);
		PSOManager::GetInstance()->SetPSO(cmd.psoDesc);
		commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// 共通バッファのセット
		commandList_->SetGraphicsRootConstantBufferView(3, lightManager_->GetLightCountBuffer().GetGPUVirtualAddress());
		commandList_->SetGraphicsRootDescriptorTable(5, SRVManager::GetInstance()->GetGPUDescriptorHandle(lightManager_->GetDirLightSrvHandle()));
		commandList_->SetGraphicsRootDescriptorTable(6, SRVManager::GetInstance()->GetGPUDescriptorHandle(lightManager_->GetPointLightSrvHandle()));
		commandList_->SetGraphicsRootDescriptorTable(7, SRVManager::GetInstance()->GetGPUDescriptorHandle(lightManager_->GetSpotLightSrvHandle()));
		commandList_->SetGraphicsRootDescriptorTable(8, SRVManager::GetInstance()->GetGPUDescriptorHandle(lightManager_->GetRectLightSrvHandle()));
		commandList_->SetGraphicsRootDescriptorTable(10, TextureManager::GetInstance()->GetTextureHandle("skybox"));

		// カメラバッファのバインド
		commandList_->SetGraphicsRootConstantBufferView(2, cameraCBVAddress);

		// コマンド固有のバッファ、ビューをセット
		//2番目のバッファビューが有効（0以外）ならカウントを2にする
		UINT vbvCount = (cmd.vbViews[1].BufferLocation != 0) ? 2 : 1;

		commandList_->IASetVertexBuffers(0, vbvCount, cmd.vbViews);
		commandList_->IASetIndexBuffer(&cmd.ibv);

		commandList_->SetGraphicsRootConstantBufferView(0, cmd.transformCBV);
		commandList_->SetGraphicsRootConstantBufferView(1, cmd.materialCBV);
		commandList_->SetGraphicsRootDescriptorTable(4, cmd.textureSRV);

		// スキニング用のSRVがあればバインド
		if(cmd.skinningSRV.ptr != 0) {
			commandList_->SetGraphicsRootDescriptorTable(9, cmd.skinningSRV);
		}

		// 5. 描画！
		commandList_->DrawIndexedInstanced(cmd.indexCount, 1, 0, 0, 0);
	}
}

void RenderSystem::ClearCommands() {
	commandQueue_.clear();
}