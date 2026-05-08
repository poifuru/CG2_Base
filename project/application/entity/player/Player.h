#pragma once
#include <list>
#include "Entity.h"
#include "CameraOrganizer.h"
#include "Bullet.h"
#include "Reticle.h"

class Player : public Entity {
public:
	Player(DxCommon* dxCommon, CameraOrganizer* camera, InputManager* input, LightManager* light);
	~Player();

	void Initialize() override;
	void Update() override;
	void Draw() override;
	void ImGui();

	//ゲッター
	EulerTransform GetTransform() { return transform_; }
	void SetVelocity(const Vector3& velocity) { velocity_ = velocity; }
	AABB GetAABB() { return aabb_; }
	std::list<std::unique_ptr<Bullet>>& GetBullets() { return bullets_; }
	
private:	// プライベート関数
	void Input();
	void CooltimeUpdate();
	void Move();
	void BulletsUpdate();
	void BulletsDraw();

private:
	//プレイヤーのパラメータ
	float speed_ = 0.0f;

	// Bullet(listで管理)
	std::list<std::unique_ptr<Bullet>> bullets_;
	float cooltime_ = 0.0f;

	// Reticle
	std::unique_ptr<Reticle> reticle_ = nullptr;

	DxCommon* dxCommon_ = nullptr;
	InputManager* input_ = nullptr;
	LightManager* light_ = nullptr;
};