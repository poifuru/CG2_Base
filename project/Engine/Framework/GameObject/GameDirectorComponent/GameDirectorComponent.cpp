#include "PCH.h"
#include "GameDirectorComponent.h"
#include "GameObject.h"
#include "BaseScene.h"
#include "NumberDrawerComponent.h"
#include "imgui.h"
#include "LogManager.h" // ログ出力用（存在すれば）

void GameDirectorComponent::Initialize() {
	if (isInitialized_) return;
	isInitialized_ = true;

	UpdateUI();
}

void GameDirectorComponent::Update() {
	// 毎フレームUI更新を呼んでも安全（値が変わったときのみ更新などの最適化も可能だが、
	// エディタ等でパラメータを変えた時に即時反映させるため、ここでは毎フレーム更新を保証）
	UpdateUI();
}

void GameDirectorComponent::ImGui() {
#ifdef USEIMGUI
	ImGui::DragInt("Target Kills", &targetKills_, 1, 1, 1000);
	ImGui::DragInt("Current Kills", &currentKills_, 1, 0, targetKills_);

	char nameBuf[128];
	strcpy_s(nameBuf, uiObjectName_.c_str());
	if (ImGui::InputText("UI Object Name", nameBuf, sizeof(nameBuf))) {
		uiObjectName_ = nameBuf;
	}

	ImGui::Text("Boss Event Triggered: %s", isBossEventTriggered_ ? "TRUE" : "FALSE");

	if (ImGui::Button("Manual Trigger Event")) {
		OnTargetKillsAchieved();
	}

	if (ImGui::Button("Reset Director")) {
		currentKills_ = 0;
		isBossEventTriggered_ = false;
		UpdateUI();
	}
#endif
}

void GameDirectorComponent::Serialize(json& j) const {
	j["type"] = "GameDirectorComponent";
	j["targetKills"] = targetKills_;
	j["currentKills"] = currentKills_;
	j["isBossEventTriggered"] = isBossEventTriggered_;
	j["uiObjectName"] = uiObjectName_;
}

void GameDirectorComponent::Deserialize(const json& j) {
	isInitialized_ = true;
	if (j.contains("targetKills")) targetKills_ = j["targetKills"];
	if (j.contains("currentKills")) currentKills_ = j["currentKills"];
	if (j.contains("isBossEventTriggered")) isBossEventTriggered_ = j["isBossEventTriggered"];
	if (j.contains("uiObjectName")) uiObjectName_ = j["uiObjectName"];
}

void GameDirectorComponent::NotifyEnemyDead() {
	if (isBossEventTriggered_) return;

	currentKills_++;
	UpdateUI();

	if (currentKills_ >= targetKills_) {
		isBossEventTriggered_ = true;
		OnTargetKillsAchieved();
	}
}

void GameDirectorComponent::OnTargetKillsAchieved() {
	// 目標撃破数を達成したときの処理！
	// TODO: ボス出現イベントムービーを流して水中フェーズに移行する
	
	// 例: コンソールやImGuiのログウィンドウに表示
#ifdef _DEBUG
	OutputDebugStringA("--- Game Director: Target Kills Achieved! Triggering Boss Spawn Event ---\n");
#endif

	// 将来、ボスカメラの起動や水中ポストエフェクトの有効化、水中BGM再生などをここに記述します。
}

void GameDirectorComponent::UpdateUI() {
	if (!gameObject_) return;
	SceneContext* context = gameObject_->GetContext();
	if (!context || !context->activeGameObjects) return;

	// UIオブジェクトを検索
	for (auto& obj : *(context->activeGameObjects)) {
		if (obj->GetName() == uiObjectName_) {
			if (auto* drawer = obj->GetComponent<NumberDrawerComponent>()) {
				int remaining = std::max(0, targetKills_ - currentKills_);
				drawer->SetValue(remaining);
				break;
			}
		}
	}
}
