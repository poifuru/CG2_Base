#include "PCH.h"
#include "DarumaTowerComponent.h"
#include "GameObject.h"
#include "MeshRendererComponent.h"
#include "InputManager.h"
#include "RawInput.h"
#include "WindowsAPI.h"

#ifdef USEIMGUI
#include "imgui.h"
#endif

void DarumaTowerComponent::Initialize() {
	// DarumaTowerManager 自体がシリアライズでダブらないよう設定
	if (gameObject_) {
		gameObject_->SetSerializable(false);
	}
	SpawnTower(initialBlockCount_);
}

void DarumaTowerComponent::SpawnTower(int blockCount) {
	if (!gameObject_) return;
	SceneContext* context = gameObject_->GetContext();
	if (!context || !context->gameObjects) return;

	// 既存のブロックを削除
	for (size_t i = 0; i < blocks_.size(); ++i) {
		if (blocks_[i].gameObject) {
			blocks_[i].gameObject->Destroy();
		}
	}
	blocks_.clear();

	Vector3 basePos = gameObject_->GetTransform().translate;

	// 1段あたりのY方向移動量（高さ * スケールY + 隙間）
	float stepY = (blockScale_.y * blockHeight_) + blockSpacing_;

	// 下から順にブロックを生成して配置
	for (int i = 0; i < blockCount; ++i) {
		std::string objName = "DarumaBlock_" + std::to_string(i);
		auto blockObj = std::make_unique<GameObject>(context, objName);
		
		// シーンファイルに自動生成ブロックが二重保存されないように設定
		blockObj->SetSerializable(false);

		// 位置の設定
		Vector3 pos = basePos;
		pos.y += static_cast<float>(i) * stepY;
		blockObj->GetTransform().translate = pos;
		blockObj->GetTransform().scale = blockScale_;

		// メッシュレンダラーコンポーネントを追加
		auto* meshRenderer = blockObj->AddComponent<MeshRendererComponent>();
		meshRenderer->Initialize();

		// 一番上（頭）と途中のブロックで色を変える
		if (i == blockCount - 1) {
			meshRenderer->SetColor(Vector4(1.0f, 0.2f, 0.2f, 1.0f)); // 赤（だるま頭）
		} else if (i % 2 == 0) {
			meshRenderer->SetColor(Vector4(0.2f, 0.7f, 1.0f, 1.0f)); // 青
		} else {
			meshRenderer->SetColor(Vector4(1.0f, 0.8f, 0.2f, 1.0f)); // 黄
		}

		DarumaBlockInfo info;
		info.gameObject = blockObj.get();
		info.targetPosition = pos;
		info.isKnockedOut = false;

		blocks_.push_back(info);

		// シーンの追加キューに移動
		context->gameObjects->push_back(std::move(blockObj));
	}

	state_ = DarumaTowerState::Idle;
}

void DarumaTowerComponent::KnockOutBottomBlock() {
	if (blocks_.empty() || state_ != DarumaTowerState::Idle) {
		return;
	}

	state_ = DarumaTowerState::KnockingOut;
	knockOutTimer_ = 0.0f;
	blocks_[0].velocity = Vector3(25.0f, 0.0f, 0.0f); // Xプラス方向へ吹き飛ばす
}

void DarumaTowerComponent::Update() {
	InputManager* input = InputManager::GetInstance();
	RawInput* rawInput = input ? input->GetRawInput() : nullptr;

	// 1/60 秒の簡易デルタタイム
	float deltaTime = 1.0f / 60.0f;

	// IDLE状態：SpaceキーまたはImGui操作で最下段を叩く
	if (state_ == DarumaTowerState::Idle) {
		if (rawInput && rawInput->Trigger(VK_SPACE)) {
			KnockOutBottomBlock();
		}
	}
	// 弾き出しアニメーション中
	else if (state_ == DarumaTowerState::KnockingOut) {
		knockOutTimer_ += deltaTime;

		if (!blocks_.empty() && blocks_[0].gameObject) {
			Vector3 currentPos = blocks_[0].gameObject->GetTransform().translate;
			currentPos.x += blocks_[0].velocity.x * deltaTime;
			blocks_[0].gameObject->GetTransform().translate = currentPos;
		}

		if (knockOutTimer_ >= knockOutDuration_) {
			// 弾かれた最下段ブロックを破棄
			if (!blocks_.empty()) {
				if (blocks_[0].gameObject) {
					blocks_[0].gameObject->Destroy();
				}
				blocks_.erase(blocks_.begin());
			}

			// 残ったブロックたちの目標Y座標を一段分下に更新
			Vector3 basePos = gameObject_->GetTransform().translate;
			float stepY = (blockScale_.y * blockHeight_) + blockSpacing_;

			for (size_t i = 0; i < blocks_.size(); ++i) {
				blocks_[i].targetPosition = basePos;
				blocks_[i].targetPosition.y += static_cast<float>(i) * stepY;
			}

			state_ = DarumaTowerState::Falling;
		}
	}
	// 落下アニメーション中
	else if (state_ == DarumaTowerState::Falling) {
		bool allArrived = true;

		for (size_t i = 0; i < blocks_.size(); ++i) {
			if (!blocks_[i].gameObject) continue;

			Vector3 currentPos = blocks_[i].gameObject->GetTransform().translate;
			float targetY = blocks_[i].targetPosition.y;

			if (currentPos.y > targetY) {
				currentPos.y -= fallSpeed_ * deltaTime;
				if (currentPos.y <= targetY) {
					currentPos.y = targetY;
				} else {
					allArrived = false;
				}
				blocks_[i].gameObject->GetTransform().translate = currentPos;
			}
		}

		if (allArrived) {
			state_ = DarumaTowerState::Idle;
		}
	}
}

void DarumaTowerComponent::ImGui() {
#ifdef USEIMGUI
	if (ImGui::TreeNode("Daruma Tower")) {
		ImGui::Text("Remaining Blocks: %d", static_cast<int>(blocks_.size()));

		const char* stateStr = "Unknown";
		if (state_ == DarumaTowerState::Idle) stateStr = "Idle (Press SPACE)";
		else if (state_ == DarumaTowerState::KnockingOut) stateStr = "Knocking Out";
		else if (state_ == DarumaTowerState::Falling) stateStr = "Falling";

		ImGui::Text("State: %s", stateStr);

		bool needRespawn = false;

		if (ImGui::SliderInt("Block Count", &initialBlockCount_, 1, 20)) {
			needRespawn = true;
		}

		if (ImGui::SliderFloat("Block Height", &blockHeight_, 0.5f, 5.0f)) {
			needRespawn = true;
		}

		if (ImGui::SliderFloat("Block Spacing", &blockSpacing_, 0.0f, 1.0f)) {
			needRespawn = true;
		}

		if (needRespawn && state_ == DarumaTowerState::Idle) {
			SpawnTower(initialBlockCount_);
		}

		if (ImGui::Button("Knock Out Bottom Block") && state_ == DarumaTowerState::Idle) {
			KnockOutBottomBlock();
		}

		if (ImGui::Button("Reset / Respawn Tower")) {
			SpawnTower(initialBlockCount_);
		}

		ImGui::TreePop();
	}
#endif
}

void DarumaTowerComponent::Serialize(json& j) const {
	j["type"] = GetName();
	j["initialBlockCount"] = initialBlockCount_;
	j["blockHeight"] = blockHeight_;
	j["blockSpacing"] = blockSpacing_;
}

void DarumaTowerComponent::Deserialize(const json& j) {
	if (j.contains("initialBlockCount")) {
		initialBlockCount_ = j["initialBlockCount"];
	}
	if (j.contains("blockHeight")) {
		blockHeight_ = j["blockHeight"];
	}
	if (j.contains("blockSpacing")) {
		blockSpacing_ = j["blockSpacing"];
	}
}
