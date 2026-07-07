#pragma once
#include "Component.h"

class FishEnemyComponent : public Component {
public:
	FishEnemyComponent() = default;
	~FishEnemyComponent() override = default;

	void Initialize() override;
	void Update() override;
	void ImGui() override;

	void Serialize(json& j) const override;
	void Deserialize(const json& j) override;

	const char* GetName() const override { return "FishEnemyComponent"; }

public:
	void OnDead();
	bool IsDead() const { return isDead_; }

private:
	Vector3 startPos_{};      // 往復の開始地点
	float moveRange_ = 10.0f; // 往復する範囲
	float speed_ = 5.0f;      // 移動スピード
	float direction_ = 1.0f;  // 現在の移動方向（1.0f: 右, -1.0f: 左）
	bool isInitialized_ = false; // リセット防止

	bool isDead_ = false;       // 死亡フラグ
	float deathTimer_ = 0.0f;   // 死亡演出タイマー
	Vector3 originalScale_{};   // 死亡時の初期スケール
};