#include "CopyImage.h"
#include "RenderTexture.h"
#include "SRVManager.h"

void CopyImageEffect::Initialize(DxCommon* dxCommon) {
	BasePostEffect::Initialize(dxCommon);
	psoDesc_.PS_ID = ShaderManager::GetInstance()->CompileAndCacheShader(
		L"Resources/shader/CopyImage.PS.hlsl", L"ps_6_0"
	);
}

void CopyImageEffect::Draw(RenderTexture* renderTexture, CameraOrganizer* /*camera*/) {
	auto cmdList = dxCommon_->GetCommandList();

	// パイプライン設定
	RootSignatureManager::GetInstance()->SetRootSignature(psoDesc_.RootSignatureID);
	PSOManager::GetInstance()->SetPSO(psoDesc_);

	// プリミティブトポロジーを設定
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// SRVをセット
	SRVManager::GetInstance()->SetGraphicsRootDescriptorTable(1, renderTexture->GetSrvIndex());

	// 描画
	cmdList->DrawInstanced(3, 1, 0, 0);
}

void CopyImageEffect::ImGui() {
	// 調整するパラメータはないので空でOK
}
