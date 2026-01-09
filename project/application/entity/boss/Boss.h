#pragma once
#include "Entity.h"
#include "CameraOrganizer.h"
#include <vector>

// 状態の定義
enum class BossState {
	Wander,    // ゆらゆら
	Dash,      // 突進
	Return,   // ← 追加：定位置に戻る状態
	Shoot      // 弾撃ち
};

class Boss : public Entity {
public:		//メンバ関数
	Boss(DxCommon* dxCommon, CameraOrganizer* camera);
	~Boss();

	void Initialize() override;
	void Update() override;
	void Draw() override;
	void IsHit(bool attackIsHit);

	void SetLight(ID3D12Resource* light) { light_ = light; }
	void SetTarget(const Transform& playerTransform) { target_ = playerTransform; }

private:
	void AABBPos();

	// --- 追加：各振る舞いの関数 ---
	void WanderBehavior();
	void DashBehavior();
	void ShootBehavior();

public:
	AABB GetAABBModel() { return aabb_; }
	bool GetIsHit() { return isHit_; }
	int GetHp() { return hp_; }

private:
	//固有ステ
	int hp_ = 0;
	bool isHit_ = false;

	CameraOrganizer* camera_ = nullptr;
	ID3D12Resource* light_ = nullptr;

	//追従するためのプレイヤーのトランスフォーム
	Transform target_ = {};

	// --- 追加：状態管理用メンバ ---
	BossState state_ = BossState::Wander;
	float stateTimer_ = 0.0f;     // 次の状態へ移るためのタイマー
	float behaviorTimer_ = 0.0f;  // 行動中の細かな制御用
	Vector3 toPlayer = {};
	Vector3 dashVelocity_ = { 0, 0, 0 }; // 突進方向
	float length = 0.0f;
	float floatingTime_ = 0.0f;   // ゆらゆら用
	float damageTimer_ = 0.0f; // 被弾後の点滅用

	const float kInitialHeight = 14.0f; // 戻りたい高さ
	const float kMaxPosX = 38.0f; // x軸の限界値
};

