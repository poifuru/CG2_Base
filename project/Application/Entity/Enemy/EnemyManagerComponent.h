#pragma once
#include "Component.h"

class EnemyManagerComponent : public Component {
public:
	EnemyManagerComponent() = default;
	~EnemyManagerComponent() override = default;

	void Initialize() override;
	void Update() override;
	void ImGui() override;

	void Serialize(json& j) const override;
	void Deserialize(const json& j) override;

	const char* GetName() const override { return "EnemyManagerComponent"; }

private:
	// 敵をスポーンさせる処理
	void SpawnEnemy();

private:
	float spawnInterval_ = 3.0f; // スポーン間隔（秒）
	float spawnTimer_ = 0.0f;    // 残り時間タイマー
	float spawnRadius_ = 20.0f;  // プレイヤーからのスポーン距離
};