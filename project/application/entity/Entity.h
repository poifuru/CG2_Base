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

protected:
	std::unique_ptr<Model> model_;
	EulerTransform transform_;
	Vector3 velocity_;
	Vector3 acceleration_;
	AABB aabb_;
	Vector3 aabbSize_;

	int hp_ = 0;
};