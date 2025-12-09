#pragma once
#include "Model.h"
#include "CenterStompCollider.h"

class Boss;

enum class StompPhase {
	None,       // 何もしていない
	Rise,       // 中央上空へ移動中
	Hover,      // 上空で待機（予兆表示）
	Fall,       // 急降下（攻撃）
	Cooldown    // 着地後の硬直
};

class CenterStomp {
public:
	CenterStomp(Boss* boss);
	~CenterStomp();

	void Initialize();
	void Update(Matrix4x4* m);
	void Draw();
	void ImGuiControl();

	void StartAttack();
	bool IsAttacking() const { return phase_ != StompPhase::None; }

	Collider* GetCollider() const { return collider_.get(); }
	Vector3 GetPosition() const { return transform_.translate; }

private:
	// 各フェーズの処理
	void UpdateRise();   // 上昇
	void UpdateHover();  // 待機
	void UpdateFall();   // 落下
	void UpdateCooldown(); // 硬直
private:
	// ボス攻撃時の波
	std::unique_ptr<Model> model_ = nullptr;
	Transform transform_;
	bool isAliveWave_ = false;

	// ボスの攻撃系
	Boss* boss_ = nullptr;
	StompPhase phase_ = StompPhase::None;
	int timer_ = 0;           // 汎用タイマー
	int duration_ = 0;        // 目標時間
	Vector3 startPos_;        // 移動開始地点
	Vector3 targetPos_;       // 移動目標地点
	const Vector3 kCenterPoint_ = {0.0f, 10.0f, 0.0f};

	std::unique_ptr<CenterStompCollider> collider_ = nullptr;
};

