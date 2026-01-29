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

protected:
	// マップチップとの当たり判定・座標補正
	void CheckMapCollision(MapChip* mapChip);

	std::unique_ptr<Model> model_;
	Transform transform_;
	Vector3 velocity_;
	Vector3 acceleration_;
	AABB aabb_;

	float friction_ = 0.85f; // 摩擦係数（慣性の残り具合）
	bool isGrounded_ = false; // 接地フラグ
	bool isTouchingWallLeft_ = false;  // 左の壁に触れているか
	bool isTouchingWallRight_ = false; // 右の壁に触れているか
};