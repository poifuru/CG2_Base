#pragma once
#include <memory>
#include "struct.h"
#include "Model.h"
#include "Mapchip.h"

class Entity {
public:
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;

	virtual void OnDamage() {} // ダメージを受けた時の処理
	
	//アクセッサ
	bool IsAlive() const { return hp_ > 0; }
	void SetAABBSize(const Vector3& size) { aabbSize_ = size; }
	bool IsGoalReached() const { return isGoalReached_; }

protected:
	// マップチップとの当たり判定・座標補正
	void CheckMapCollision(MapChip* mapChip); 
	void UpdateAABB();

	std::unique_ptr<Model> model_;
	Transform transform_;
	Vector3 velocity_;
	AABB aabb_;
	Vector3 aabbSize_;

	bool isGrounded_ = false; // 接地フラグ
	bool isTouchingWallLeft_ = false;  // 左の壁に触れているか
	bool isTouchingWallRight_ = false; // 右の壁に触れているか
	bool isOnDamageFloor_ = false; // ダメージ床に乗っているかフラグ
	int hp_ = 0;
	bool isGoalReached_ = false; // ゴールに到達したかフラグ
};