#include "PCH.h"
#include "EnemyManagerComponent.h"
#include "GameObject.h"
#include "DeltaTime.h"
#include "BirdEnemyComponent.h"
#include "FishEnemyComponent.h"
#include "ColliderComponent.h"
#include "BaseScene.h" // SceneContext や gameObjects へのアクセス用
#include "PlayerComponent.h" // プレイヤー検索用
#include "MeshRendererComponent.h"

void EnemyManagerComponent::Initialize() {
	if (isInitialized_) return;

	isInitialized_ = true;
	// タイマーの初期化
	spawnTimer_ = spawnInterval_;
}

void EnemyManagerComponent::Update() {
	if (!gameObject_) return;

	// 時間経過でタイマーを減算
	spawnTimer_ -= kDeltaTime;
	if (spawnTimer_ <= 0.0f) {
		spawnTimer_ = spawnInterval_;

		SpawnEnemy();
	}
}

void EnemyManagerComponent::SpawnEnemy() {
	auto* context = gameObject_->GetContext();

	if (!context || !context->gameObjects || !context->activeGameObjects) return;

	// 基準となるプレイヤーの位置を探す
	Vector3 playerPos = { 0.0f, 0.0f, 0.0f };
	bool foundPlayer = false;

	for (const auto& obj : *(context->activeGameObjects)) {
		if (obj->GetName() == "Player" || obj->GetComponent<PlayerComponent>() != nullptr) {
			playerPos = obj->GetTransform().translate;
			foundPlayer = true;

			break;
		}
	}

	// プレイヤーが見つからない場合はスポーンさせない
	if (!foundPlayer) return;

	// プレイヤーの周囲のランダムな位置をスポーン座標にする
	float angle = static_cast<float>(rand()) / RAND_MAX * 3.14159265f * 2.0f;
	Vector3 spawnPos = {
		playerPos.x + spawnRadius_ * std::cos(angle),
		playerPos.y, // 高さはプレイヤーと同じ
		playerPos.z + spawnRadius_ * std::sin(angle)
	};

	// 敵のタイプをランダムで決定 (0: 鳥, 1: 魚)
	int enemyType = rand() % 2;
	auto enemyObj = std::make_unique<GameObject>(context, "Enemy");

	if (enemyType == 0) {
		// --- 鳥エネミーの生成 ---
		auto* mesh = enemyObj->AddComponent<MeshRendererComponent>();
		mesh->SetModel("Resources/Enemy/Bird/bird.obj");
		mesh->SetTexture("white1x1");
		// スケール
		enemyObj->GetTransform().scale = { 0.2f, 0.2f, 0.2f };
		// 挙動とコライダーを追加
		enemyObj->AddComponent<BirdEnemyComponent>();
		auto* collider = enemyObj->AddComponent<ColliderComponent>();
		// 初期座標を適用
		enemyObj->GetTransform().translate = spawnPos;
		enemyObj->GetTransform().translate.y = spawnPos.y + 4.0f;	// y座標をちょっと高めに
		// コンポーネントをすべて追加した後に初期化を呼ぶ
		enemyObj->Initialize();
		// 初期化完了後にコライダーの半径を設定（デフォルト値を上書き）
		collider->SetRadius(1.0f);
	}
	else {
		// --- 魚エネミーの生成 ---
		auto* mesh = enemyObj->AddComponent<MeshRendererComponent>();
		mesh->SetModel("Resources/Enemy/smallFish/smallFish.obj");
		mesh->SetTexture("white1x1");
		// スケール
		enemyObj->GetTransform().scale = { 1.0f, 1.0f, 1.0f };
		// 挙動とコライダーを追加
		enemyObj->AddComponent<FishEnemyComponent>();
		auto* collider = enemyObj->AddComponent<ColliderComponent>();
		// 初期座標を適用
		enemyObj->GetTransform().translate = spawnPos;
		// コンポーネントをすべて追加した後に初期化を呼ぶ
		enemyObj->Initialize();
		// 初期化完了後にコライダーの半径を設定
		collider->SetRadius(1.0f);
	}
	// 動的生成なのでセーブ対象外に
	enemyObj->SetSerializable(false);

	// シーンのオブジェクトリストに追加
	context->gameObjects->push_back(std::move(enemyObj));
}

void EnemyManagerComponent::ImGui() {
	ImGui::DragFloat("Spawn Interval", &spawnInterval_, 0.1f, 0.5f, 60.0f);
	ImGui::DragFloat("Spawn Radius", &spawnRadius_, 0.5f, 5.0f, 100.0f);
}

void EnemyManagerComponent::Serialize(json& j) const {
	j["type"] = "EnemyManagerComponent";
	j["spawnInterval"] = spawnInterval_;
	j["spawnRadius"] = spawnRadius_;
}

void EnemyManagerComponent::Deserialize(const json& j) {
	isInitialized_ = true;
	if (j.contains("spawnInterval")) spawnInterval_ = j["spawnInterval"];
	if (j.contains("spawnRadius")) spawnRadius_ = j["spawnRadius"];
}