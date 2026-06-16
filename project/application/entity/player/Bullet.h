#pragma once
#include "Entity.h"
#include "CameraOrganizer.h"

class BaseEnemy;
class Player;

class Bullet : public Entity {
public:
	Bullet(DxCommon* dxCommon, CameraOrganizer* camera, InputManager* input, LightManager* light);

	~Bullet();

	void Initialize() override;

	void Update() override;

	void Draw() override;

	void ImGui();

	// アクセッサ
	bool IsActive() { return isActive_; }
	const AABB& GetAABB() const { return aabb_; }
	void SetIsActive(bool flag) { isActive_ = flag; }
	void SetDirection(const Vector3& direction) { direction_ = direction; }
	void SetTarget(BaseEnemy* target) { target_ = target; }
	void SetPlayer(const Player* player) { player_ = player; }

private:
	void Move();
	void Life();

private:
	bool isActive_ = true;
	float activeTimer_ = 0.0f;
	float speed_ = 0.0f;
	Vector3 direction_{};
	BaseEnemy* target_ = nullptr;
	const Player* player_ = nullptr;
	float homingStrength_ = 5.0f; // ホーミングの強度

	// デバッグ用
	bool hitbox_ = false;
	Vector4 hitboxColor_{};
};