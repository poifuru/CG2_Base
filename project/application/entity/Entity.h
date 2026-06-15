#pragma once
#include <memory>
#include "struct.h"
#include "Model.h"
#include "CameraOrganizer.h"
#include "GameObject.h"

class Entity : public GameObject {
public:
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;

	virtual void OnDamage() {} // ダメージを受けた時の処理

	//アクセッサ
	void SetVelocity(const Vector3& velocity) { velocity_ = velocity_; }
	void SetAcceleration(const Vector3& acceleration) { acceleration_ = acceleration; }
	void SetAABBSize(const Vector3& size) { aabbSize_ = size; }

protected:
	void DrawHitbox(Vector4 color);

protected:
	std::unique_ptr<Model> model_;
	Vector3 velocity_{};
	Vector3 acceleration_{};
	AABB aabb_{};
	Vector3 aabbSize_{};

	CameraOrganizer* camera_ = nullptr;
};