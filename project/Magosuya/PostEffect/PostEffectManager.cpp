#include "PostEffectManager.h"
#include "DxCommon.h"
#include "BasePostEffect.h"
#include "RenderTexture.h"
#include "SRVManager.h"

void PostEffectManager::Initialize(DxCommon* dxCommon, uint32_t windowWidth, uint32_t windowHeight) {
	dxCommon_ = dxCommon;

	for(int i = 0; i < 2; ++i) {
		workTextures_[i] = std::make_unique<RenderTexture>();
		workTextures_[i]->Initialize(dxCommon, SRVManager::GetInstance());
	}
}

void PostEffectManager::Finalize() {

}

void PostEffectManager::AddEffect(BasePostEffect* effect) {

}

void PostEffectManager::ClearEffects() {

}

void PostEffectManager::Execute(RenderTexture* srcTexture) {
	if(effects_.empty()) { return; }

	auto cmdList = dxCommon_->GetCommandList();

	// 現在の入力を指すポインタ(初期状態はゲーム画面のテクスチ)
	RenderTexture* currentInput = srcTexture;

	// ワークバッファのインデックス(0, 1を交互に入れ替えていく)
	int currentTargetIndex = 0;

	// 登録されたエフェクトをループで回す
	for(size_t i = 0; i < effects_.size(); ++i) {
		if(effects_[i] == nullptr) continue;

		if(!effects_[i]->GetIsActive())	continue;

		// 最後の1個かどうかの判定
		bool isLast = (i == effects_.size() - 1);

		// 出力先を中間バッファに切り替える
		RenderTexture* nextOutput = workTextures_[currentTargetIndex].get();

		// バリアを貼る
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = nextOutput->GetResource();	// リソースを取得
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		cmdList->ResourceBarrier(1, &barrier);

		// レンダーターゲットを設定(中間バッファのRTVをセット)
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = nextOutput->GetDescriptorHandle();
		cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

		// 画面のクリア処理
		float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

		// 描画の実行
		// curerntInput をテクスチャとして読み込んで描画
		effects_[i]->Draw(currentInput);

		if(!isLast) {
			// 描画が終わったので今書きこんだ中間バッファを次の入力用(読み取り用)として使うために
			// 「RENDER_TARGET」から「PIXEL_SHADER_RESOURCE」に戻すバリアを貼る
			RenderTexture* justRenderOutput = workTextures_[currentTargetIndex].get();

			D3D12_RESOURCE_BARRIER barrier{};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Transition.pResource = justRenderOutput->GetResource();	// リソースを取得
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			cmdList->ResourceBarrier(1, &barrier);

			// 次のループのために今回の出力を「次の入力」に設定
			currentInput = justRenderOutput;

			// ターゲットのインデックスを反転させる(0なら1、1なら0)
			currentTargetIndex = 1 - currentTargetIndex;
		}
	}
}

void PostEffectManager::ImGuiUpdate() {

}
