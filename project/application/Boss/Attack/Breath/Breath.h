#pragma once
#include "MagosuyaEngine.h"
#include "object/3d/Model.h"
#include "../AttackPhase.h"
#include <vector>
#include <memory>

class Boss;

// 弾（子機）の情報を持つ構造体
struct MinionProjectile {
	Transform transform;
	Vector3 velocity;     // 速度
	float lifeTime = 0.0f;
	bool isActive = false;
	std::unique_ptr<Model> model = nullptr;
	// カーブ関連のメンバーをすべて削除
	float curveForce = 0.0f;
	int currentCurveTimer = 0;
	bool isCurvingZ = false;
};

class Breath {
public:
	// 弾の数を調整 (例: 100個に増やし、連続攻撃に対応できるようにする)
	static const int kNumProjectiles = 1000;
	// 弾の最大ライフタイム (例: 5秒 = 300フレーム)
	static const int kMaxLifeTime = 300;

	Breath(DxCommon* dxCommon, Boss* boss);
	~Breath();

	void Initialize();
	// ボスから受け取った親行列でモデルを更新
	void Update(Matrix4x4* m, Vector3 target);
	void Draw();
	void ImGuiControl();

	// 連続攻撃の回数と間隔を引数に追加
	void StartAttack(int numThrows, float intervalSeconds);
	// AttackPhase::None が別途定義されている前提
	bool IsAttacking() const { return phase_ != AttackPhase::None; }

private:
	// 各フェーズの処理
	void UpdateCharge();
	void UpdateShoot();
	void UpdateCooldown();

	// 弾の更新処理
	void UpdateProjectiles();
	// 弾を発射する処理
	void EmitProjectiles();

private:
	DxCommon* dxCommon_ = nullptr;
	Boss* boss_ = nullptr;

	// AttackPhase::None が別途定義されている前提
	AttackPhase phase_ = AttackPhase::None;
	int timer_ = 0;           // 汎用タイマー
	int duration_ = 0;        // 目標時間

	MinionProjectile projectiles_[kNumProjectiles];

	// 連続攻撃の管理
	int totalThrows_ = 0;      // 投げる総回数 (引数で設定)
	int throwCount_ = 0;       // 現在投げた回数
	int intervalFrames_ = 0;   // 投げる間隔 (フレーム数)
	int intervalTimer_ = 0;    // 次に投げるまでのタイマー
	// bool isCurving_ = false;   // 削除
	Vector3 targetPos_ = { 0.0f, 0.0f, 0.0f }; // ターゲット座標

	// 弾のスピードとカーブの力
	const float kProjectileSpeed_ = 0.3f;
	
	Matrix4x4* vp_ = nullptr;
};