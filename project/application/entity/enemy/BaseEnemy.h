#pragma once
#include "Entity.h"
#include <memory>
#include "Model.h"

class BaseEnemy : public Entity {
public:
	BaseEnemy(DxCommon* dxCommon, LightManager* light, MapChip* mapchip);
	virtual ~BaseEnemy();

	void Initialize() override;
	void Update() override;
	void Draw() override;
	void ImGui();

	// 攻撃が当たった時の処理
	void OnHit(int damage, const Vector3& playerPos);

	// ゲッター
	const AABB& GetAABB() const { return aabb_; }
	Transform GetTransform() { return transform_; }
	void SetPosition(const Vector3& pos) { transform_.translate = pos; }
	void SetAABBSize(const Vector3& size) { aabbSize_ = size; }
	// 完全に消滅したか（PlaySceneでリストから消す用）
	bool IsDead() const { return isDead_; }

protected:
	MapChip* mapchip_ = nullptr;
	// 敵固有の動き（左右往復など）のための変数
	float walkSpeed_;
	// --- 無敵時間関連 ---
	int invincibleTimer_ = 0;      // 残りの無敵フレーム
	const int kInvincibleTime = 20; // 被弾後の無敵時間（0.33秒くらい）

	//死亡演出用
	bool isDead_ = false;          // 死亡演出中フラグ
	float alpha_ = 1.0f;           // 透明度 (1.0で不透明、0.0で透明)
	const float kFadeSpeed = 0.05f; // 消える速さ（毎フレーム 0.05 減らす）
};