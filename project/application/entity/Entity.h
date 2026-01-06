#pragma once
#include <memory>
#include "struct.h"
#include "Model.h"

class Entity {
public:
	virtual void Initialize();
	virtual void Update();
	virtual void Draw();

protected:
	std::unique_ptr<Model> model_;
	AABB aabb_;
	Transform transform_;
	Vector3 velocity_;
};