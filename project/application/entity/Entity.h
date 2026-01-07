#pragma once
#include <memory>
#include "struct.h"
#include "Model.h"

class Entity {
public:
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;

protected:
	std::unique_ptr<Model> model_;
	AABB aabb_;
	Transform transform_;
	Vector3 velocity_;
};