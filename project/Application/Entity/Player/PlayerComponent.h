#pragma once
#include "Component.h"

class PlayerComponent : public Component {
public:
	PlayerComponent() = default;
	~PlayerComponent() override = default;

	// Componentのライフサイクル関数
	void Initialize() override;
	void Update() override;
	void ImGui() override;

	//ゲッター
	const char* GetName() const override { return "PlayerComponent"; }
	
private:	// プライベート関数
	void Move();  // 移動処理
	void Shoot(); // 弾の発射処理

private:
	// プレイヤーのパラメータ
	float speed_ = 1.5f;
	Vector3 velocity_{};
	Vector3 acceleration_{};
	Vector3 localTranslate_{}; // レール上でのローカル位置
	float cooltime_ = 0.0f;
	const RailPath* railPath_ = nullptr;
};