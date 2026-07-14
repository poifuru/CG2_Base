#include "PCH.h"
#include "FishEnemyComponent.h"
#include "GameObject.h"
#include "DeltaTime.h"
#include "MeshRendererComponent.h"
#include "../../../../Engine/Editor/ParticleEditor/ParticleSpawner.h"
#include "GameDirectorComponent.h"
#include "BaseScene.h"
#include "RenderingModel.h"

void FishEnemyComponent::Initialize() {
	if (isInitialized_) return;
	isInitialized_ = true;

	if (gameObject_) {
		startPos_ = gameObject_->GetTransform().translate; // 配置された初期位置を開始地点にする
	}

	moveRange_ = 10.0f;
	speed_ = 5.0f;
	direction_ = 1.0f;
}
void FishEnemyComponent::Update() {
	if (!gameObject_) return;

	// 死亡演出の更新
	if (isDead_) {
		deathTimer_ += kDeltaTime;
		const float kDeathDuration = 1.0f; // 1.0秒で消滅
		float progress = deathTimer_ / kDeathDuration;
		if (progress >= 1.0f) {
			gameObject_->Destroy();
			return;
		}

		// スケールアウト
		float scaleFactor = 1.0f - progress;
		gameObject_->GetTransform().scale = {
			originalScale_.x * scaleFactor,
			originalScale_.y * scaleFactor,
			originalScale_.z * scaleFactor
		};

		// フェードアウト
		if (auto* mesh = gameObject_->GetComponent<MeshRendererComponent>()) {
			mesh->SetBlendMode(MyEngine::Rendering::BlendModeType::Alpha);
			mesh->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f - progress });
		}
		return;
	}

	auto& trans = gameObject_->GetTransform();

	// 左右に移動
	trans.translate.x += speed_ * direction_ * kDeltaTime;

	// 指定範囲を超えたら方向を反転
	float diff = trans.translate.x - startPos_.x;
	if (std::abs(diff) > moveRange_) {
		trans.translate.x = startPos_.x + (moveRange_ * (direction_ > 0.0f ? 1.0f : -1.0f));
		direction_ *= -1.0f;
	}

	// 進行方向に向くように回転（Yaw）を設定
	trans.rotate.y = (direction_ > 0.0f) ? 1.570796f : -1.570796f; // ※モデルの初期向きで調整してね！
}
void FishEnemyComponent::ImGui() {
	ImGui::DragFloat3("Start Pos", &startPos_.x, 0.1f);
	ImGui::DragFloat("Move Range", &moveRange_, 0.1f, 0.0f, 100.0f);
	ImGui::DragFloat("Speed", &speed_, 0.1f, 0.0f, 50.0f);
}
void FishEnemyComponent::Serialize(json& j) const {
	j["type"] = "FishEnemyComponent";
	j["startPos"] = { startPos_.x, startPos_.y, startPos_.z };
	j["moveRange"] = moveRange_;
	j["speed"] = speed_;
	j["direction"] = direction_;
}
void FishEnemyComponent::Deserialize(const json& j) {
	isInitialized_ = true;
	if (j.contains("startPos")) {
		startPos_ = { j["startPos"][0], j["startPos"][1], j["startPos"][2] };
	}
	if (j.contains("moveRange")) moveRange_ = j["moveRange"];
	if (j.contains("speed")) speed_ = j["speed"];
	if (j.contains("direction")) direction_ = j["direction"];
}

void FishEnemyComponent::OnDead() {
	if (isDead_) return;
	isDead_ = true;
	deathTimer_ = 0.0f;
	if (gameObject_) {
		originalScale_ = gameObject_->GetTransform().scale;

		// 被弾位置に爆発パーティクルを生成
		ParticleSpawner::SpawnExplosion(gameObject_->GetContext(), gameObject_->GetTransform().translate, 15);

		// GameDirectorへの撃破通知
		if (gameObject_->GetContext() && gameObject_->GetContext()->activeGameObjects) {
			for (auto& obj : *(gameObject_->GetContext()->activeGameObjects)) {
				if (auto* director = obj->GetComponent<GameDirectorComponent>()) {
					director->NotifyEnemyDead();
					break;
				}
			}
		}
	}
}