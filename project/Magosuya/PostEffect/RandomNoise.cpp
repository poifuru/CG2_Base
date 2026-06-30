#include "RandomNoise.h"
#include "CameraOrganizer.h"
#include "SRVManager.h"
#include "imgui.h"

void RandomNoise::Initialize(DxCommon* dxCommon) {
	// 基底クラスの初期化（PSOの共通設定などを行う）
	BasePostEffect::Initialize(dxCommon);
	// 今回作成したピクセルシェーダーを登録
	psoDesc_.PS_ID = ShaderManager::GetInstance()->CompileAndCacheShader(
		L"Resources/shader/RandomNoise.PS.hlsl", L"ps_6_0"
	);
	// 定数バッファの作成とマッピング
	constantBuffer_ = dxCommon_->CreateBufferResource(sizeof(RandomNoiseForGPU));
	constantBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&cpuData_));
	// 初期値の設定
	cpuData_->time = 0.0f;
	cpuData_->intensity = 0.15f; // デフォルトのノイズの強さ
	// SRVスロットを確保
	srvIndexSceneColor_ = SRVManager::GetInstance()->Allocate();
}

void RandomNoise::Draw(RenderTexture* renderTexture, CameraOrganizer* camera) {
	// 毎フレーム時間を進める（ここでは簡易的に 1/60 秒ずつプラス）
	cpuData_->time += 1.0f / 60.0f;
	if (cpuData_->time > 1000.0f) {
		cpuData_->time = 0.0f; // 数値が大きくなりすぎないようにリセット
	}
	auto cmdList = dxCommon_->GetCommandList();
	auto srvManager = SRVManager::GetInstance();
	// 入力レンダーテクスチャからSRVを作成
	srvManager->CreateSRVforRenderTexture(
		srvIndexSceneColor_,
		renderTexture->GetResource(),
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
	);
	// ルートシグネチャとPSOを設定
	RootSignatureManager::GetInstance()->SetRootSignature(psoDesc_.RootSignatureID);
	PSOManager::GetInstance()->SetPSO(psoDesc_);
	// プリミティブトポロジーを設定
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// 定数バッファを設定 (b0)
	cmdList->SetGraphicsRootConstantBufferView(0, constantBuffer_->GetGPUVirtualAddress());
	// 入力テクスチャを設定 (t0)
	srvManager->SetGraphicsRootDescriptorTable(1, srvIndexSceneColor_);
	// 全画面に板ポリを描画（頂点は Fullscreen.VS.hlsl が生成してくれる）
	cmdList->DrawInstanced(3, 1, 0, 0);
}

void RandomNoise::ImGui() {
	// ImGuiから強度をリアルタイム調整できるようにする
	ImGui::DragFloat("Intensity##Noise", &cpuData_->intensity, 0.005f, 0.0f, 1.0f, "%.3f");
}