#include "PostEffectManager.h"
#include "DxCommon.h"
#include "Outline.h"
#include "ColorGrading.h"
#include "Fog.h"
#include "Vignette.h"
#include "RadialBlur.h"
#include "Dissolve.h"
#include "RandomNoise.h"
#include "CopyImage.h"
#include "RenderTexture.h"
#include "SRVManager.h"
#include "imgui.h"

PostEffectManager* PostEffectManager::GetInstance() {
	static PostEffectManager instance;
	return &instance;
}

void PostEffectManager::Initialize(DxCommon* dxCommon, uint32_t windowWidth, uint32_t windowHeight) {
	dxCommon_ = dxCommon;

	for(int i = 0; i < 2; ++i) {
		workTextures_[i] = std::make_unique<RenderTexture>();
		workTextures_[i]->Initialize(dxCommon, SRVManager::GetInstance());
	}

	// ポストエフェクトをすべて初期化
	// 最初はすべて非アクティブ状態にしておく
	effects_[static_cast<size_t>(PostEffectType::Outline)] = std::make_unique<Outline>();
	effects_[static_cast<size_t>(PostEffectType::ColorGrading)] = std::make_unique<ColorGrading>();
	effects_[static_cast<size_t>(PostEffectType::Fog)] = std::make_unique<Fog>();
	effects_[static_cast<size_t>(PostEffectType::Vignette)] = std::make_unique<Vignette>();
	effects_[static_cast<size_t>(PostEffectType::RadialBlur)] = std::make_unique<RadialBlur>();
	effects_[static_cast<size_t>(PostEffectType::Dissolve)] = std::make_unique<Dissolve>();
	effects_[static_cast<size_t>(PostEffectType::RandomNoise)] = std::make_unique<RandomNoise>();

	for (size_t i = 0; i < static_cast<size_t>(PostEffectType::Count); ++i) {
		if (effects_[i] != nullptr) {
			effects_[i]->Initialize(dxCommon_);
			effects_[i]->SetIsActive(false); // 初期状態はOFF
		}
	}

	// スルーパス用エフェクトの初期化
	copyImage_ = std::make_unique<CopyImageEffect>();
	copyImage_->Initialize(dxCommon_);
}

void PostEffectManager::Finalize() {

}

PostEffectManager::~PostEffectManager() = default;

void PostEffectManager::SetEffectActive(PostEffectType type, bool flag) {
	size_t index = static_cast<size_t>(type);
	if (effects_[index] != nullptr) {
		effects_[index]->SetIsActive(flag);
	}
}

void PostEffectManager::ClearEffects() {
	// インスタンスは消さずに、フラグをすべて false にリセットするだけ
	for (size_t i = 0; i < static_cast<size_t>(PostEffectType::Count); ++i) {
		if (effects_[i] != nullptr) {
			effects_[i]->SetIsActive(false);
		}
	}
}

void PostEffectManager::Execute(RenderTexture* srcTexture, CameraOrganizer* camera) {
	auto cmdList = dxCommon_->GetCommandList();

	// 今回のフレームで「実際に描画する、最後のアクティブなエフェクト」のインデックスを探す
	int lastActiveIndex = -1;
	for (int i = static_cast<int>(PostEffectType::Count) - 1; i >= 0; --i) {
		if (effects_[i] != nullptr && effects_[i]->GetIsActive()) {
			lastActiveIndex = i;
			break; // 末尾から探して最初に見つかったものが「最後のエフェクト」
		}
	}

	// アクティブなエフェクトが1つもない場合
	if (lastActiveIndex == -1) {
		// レンダーターゲットをバックバッファに設定する
		D3D12_CPU_DESCRIPTOR_HANDLE backBufferRtv = dxCommon_->GetCurrentBackBufferRtvHandle();
		cmdList->OMSetRenderTargets(1, &backBufferRtv, FALSE, nullptr);

		// スルーパス描画でゲーム画面をそのままバックバッファに描画
		copyImage_->Draw(srcTexture, camera);
		return;
	}

	// 現在の入力を指すポインタ(初期状態はゲーム画面のテクスチャ)
	RenderTexture* currentInput = srcTexture;
	int currentTargetIndex = 0;

	// 2. 固定配列を順番に走査して描画していく
	for (size_t i = 0; i < static_cast<size_t>(PostEffectType::Count); ++i) {
		if (effects_[i] == nullptr || !effects_[i]->GetIsActive()) {
			continue; // OFFになっているエフェクトは、存在自体を完全に無視してスキップ
		}

		// これが「今回有効なエフェクトの中での最後の1個」かどうかの判定
		bool isLast = (static_cast<int>(i) == lastActiveIndex);

		if (isLast) {
			// 【最後のエフェクトの場合】出力先は本物の画面（バックバッファ）
			D3D12_CPU_DESCRIPTOR_HANDLE backBufferRtv = dxCommon_->GetCurrentBackBufferRtvHandle();
			cmdList->OMSetRenderTargets(1, &backBufferRtv, FALSE, nullptr);
		}
		else {
			// 【途中のエフェクトの場合】出力先を中間バッファに切り替える
			RenderTexture* nextOutput = workTextures_[currentTargetIndex].get();

			D3D12_RESOURCE_BARRIER barrier{};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Transition.pResource = nextOutput->GetResource();
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			cmdList->ResourceBarrier(1, &barrier);

			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = nextOutput->GetDescriptorHandle();
			cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

			float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
			cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		}

		// 描画の実行
		effects_[i]->Draw(currentInput, camera);

		if (!isLast) {
			// 次のエフェクトのために、今書き込んだ中間バッファを読込用にバリア遷移
			RenderTexture* justRenderOutput = workTextures_[currentTargetIndex].get();

			D3D12_RESOURCE_BARRIER barrier{};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Transition.pResource = justRenderOutput->GetResource();
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			cmdList->ResourceBarrier(1, &barrier);

			// 入力ソースをこの中間バッファに更新し、ピンポンインデックスを入れ替え
			currentInput = justRenderOutput;
			currentTargetIndex = 1 - currentTargetIndex;
		}
	}
}

void PostEffectManager::ImGui() {
#ifdef USEIMGUI
	// ポストエフェクト全体の管理ウィンドウを開く
	ImGui::Begin("PostEffect Manager");

	// 全エフェクトの ON/OFF チェックボックスを一覧表示する
	ImGui::Text("Active Switches");

	for (size_t i = 0; i < static_cast<size_t>(PostEffectType::Count); ++i) {
		if (effects_[i] == nullptr) continue;

		// 列挙型に応じて表示する名前を決定する
		const char* effectName = "Unknown";
		switch (static_cast<PostEffectType>(i)) {
		case PostEffectType::Outline:      effectName = "Outline";       break;
		case PostEffectType::ColorGrading: effectName = "Color Grading"; break;
		case PostEffectType::Fog:		   effectName = "Fog";			 break;
		case PostEffectType::Vignette:     effectName = "Vignette";      break;
		case PostEffectType::RadialBlur:   effectName = "Radial Blur";   break;
		case PostEffectType::Dissolve:     effectName = "Dissolve";      break;
		case PostEffectType::RandomNoise:  effectName = "Random Noise";  break;
		}

		// 現在のフラグ状態を取得
		bool isActive = effects_[i]->GetIsActive();

		// チェックボックスを表示（クリックされたら自動で isActive が書き換わる）
		if (ImGui::Checkbox(effectName, &isActive)) {
			effects_[i]->SetIsActive(isActive);
		}
	}

	ImGui::Separator();
	ImGui::Text("Parameters");

	// ON になっているエフェクトの内部パラメータだけを下に展開する
	for (size_t i = 0; i < static_cast<size_t>(PostEffectType::Count); ++i) {
		// ここでは「実体が存在する」かつ「ONになっている」ものだけパラメータ調整を表示
		if (effects_[i] != nullptr && effects_[i]->GetIsActive()) {

			// ImGui の折りたたみヘッダー（CollapsingHeader）を使うと画面がスッキリしておすすめ
			const char* headerName = "Unknown Param";
			switch (static_cast<PostEffectType>(i)) {
			case PostEffectType::Outline:      headerName = "Outline Settings";       break;
			case PostEffectType::ColorGrading: headerName = "Color Grading Settings"; break;
			case PostEffectType::Fog:		   headerName = "Fog Settings";			  break;
			case PostEffectType::Vignette:     headerName = "Vignette Settings";      break;
			case PostEffectType::RadialBlur:   headerName = "Radial Blur Settings";   break;
			case PostEffectType::Dissolve:     headerName = "Dissolve Settings";      break;
			case PostEffectType::RandomNoise:  headerName = "Random Noise Settings";  break;
			}

			if (ImGui::CollapsingHeader(headerName, ImGuiTreeNodeFlags_DefaultOpen)) {
				// 各クラス固有の DragFloat とかを呼び出す
				effects_[i]->ImGui();
			}
		}
	}

	ImGui::End(); // ポストエフェクト管理ウィンドウを閉じる
#endif
}
