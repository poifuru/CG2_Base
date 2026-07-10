#include "RenderSystem.h"
#include "SRVManager.h"
#include "TextureManager.h"
#include "DxCommon.h"
#include "LightManager.h"
#include <algorithm>

void RenderSystem::Initialize(DxCommon* dxCommon) {
	dxCommon_ = dxCommon;
	commandList_ = dxCommon->GetCommandList();

	// コマンドシグネチャの作成 (DrawIndexed用)
	D3D12_INDIRECT_ARGUMENT_DESC argumentDesc = {};
	argumentDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

	D3D12_COMMAND_SIGNATURE_DESC signatureDesc = {};
	signatureDesc.ByteStride = 20; // sizeof(D3D12_DRAW_INDEXED_ARGUMENTS) は 20 バイト
	signatureDesc.NumArgumentDescs = 1;
	signatureDesc.pArgumentDescs = &argumentDesc;
	signatureDesc.NodeMask = 0;

	HRESULT hr = dxCommon_->GetDevice()->CreateCommandSignature(
		&signatureDesc,
		nullptr,
		IID_PPV_ARGS(&indirectCommandSignature_)
	);
	assert(SUCCEEDED(hr));
}

void RenderSystem::PushCommand(const RenderCommand& command) {
	commandQueue_.push_back(command);
}

void RenderSystem::ExecuteCommands() {
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

		// メッシュ設定
		UINT vbvCount = (cmd.vbViews[1].BufferLocation != 0) ? 2 : 1;
		commandList_->IASetVertexBuffers(0, vbvCount, cmd.vbViews);
		commandList_->IASetIndexBuffer(&cmd.ibv);

		for(UINT i = 0; i < kMaxRootParameters; ++i) {
			if(cmd.renderType != RenderType::Skybox && cmd.renderType != RenderType::Particle) {
				if(i == 0) {	// カメラバッファ
					commandList_->SetGraphicsRootConstantBufferView(i, cameraCBVAddress_);
				}
				if(i == 1) {	// ライトバッファ
					commandList_->SetGraphicsRootConstantBufferView(i, lightCBVAddress_);
				}
				if(i == 2) {
					commandList_->SetGraphicsRootDescriptorTable(i, skyboxTextureHandle_);
				}
			}

			const auto& binding = cmd.binds[i];
			if(binding.type == BindingType::None) continue;

			if(binding.type == BindingType::CBV) {
				commandList_->SetGraphicsRootConstantBufferView(i, binding.gpuAddress);
			}
			else if(binding.type == BindingType::SRV_Table) {
				commandList_->SetGraphicsRootDescriptorTable(i, binding.descriptorHandle);
			}
		}

		// 描画！
		if (cmd.useIndirect) {
			assert(cmd.indirectArgumentBuffer != nullptr);
			commandList_->ExecuteIndirect(
				indirectCommandSignature_.Get(),
				1,
				cmd.indirectArgumentBuffer,
				0,
				nullptr,
				0
			);
		} else {
			commandList_->DrawIndexedInstanced(cmd.indexCount, cmd.instanceCount, 0, 0, 0);
		}
	}
}

void RenderSystem::ClearCommands() {
	commandQueue_.clear();
}