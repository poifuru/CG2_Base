#pragma once
#include "MagosuyaEngine.h"
#include "object/3d/Model.h"
#include "../AttackPhase.h"
#include <vector>
#include <memory> // std::unique_ptrのため
#include "BossProjectileCollider.h"
#include "Audio.h"

class Boss;

// 発射する弾の情報を持つ構造体
struct Projectile {
	Transform transform;
	Vector3 velocity;
	float lifeTime = 0.0f;
	bool isActive = false;
	// ★ 弾ごとに固有のモデルポインタを持たせる
	std::unique_ptr<Model> model = nullptr;
	std::unique_ptr<BossProjectileCollider> collider = nullptr;
};

class FullScreenAttack {
public:
	// 弾の数を定義
	static const int kNumProjectiles = 16;
	// 弾の最大ライフタイム (例: 3秒 = 3 * 60フレーム)
	static const int kMaxLifeTime = 180;

	FullScreenAttack(DxCommon* dxCommon, Boss* boss);
	~FullScreenAttack();

	void Initialize();
	// ボスから受け取った親行列でモデルを更新
	void Update(Matrix4x4* m);
	void Draw();
	void ImGuiControl();

	void StartAttack();
	bool IsAttacking() const { return phase_ != AttackPhase::None; }

	std::vector<Collider*> GetColliders();

private:
	// 各フェーズの処理
	void UpdateCharge();
	void UpdateShoot();
	void UpdateCooldown();

	// 弾の更新処理
	void UpdateProjectiles();
	// 16個の弾を発射する処理
	void EmitProjectiles();

private:
	DxCommon* dxCommon_ = nullptr;
	Boss* boss_ = nullptr;

	AttackPhase phase_ = AttackPhase::None;
	int timer_ = 0;           // 汎用タイマー
	int duration_ = 0;        // 目標時間

	// 以前の単一モデルポインタは削除

	// 16方向の弾を管理する配列
	Projectile projectiles_[kNumProjectiles];

	// 弾のスピード (必要に応じて調整)
	const float kProjectileSpeed_ = 0.5f;

	Matrix4x4* vp_ = nullptr;

	Audio audio_;
	SoundData breathHandle_;
};