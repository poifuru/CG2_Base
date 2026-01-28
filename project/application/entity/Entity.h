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
	AABB aabb_;
};