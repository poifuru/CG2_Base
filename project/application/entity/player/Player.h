#pragma once
#include <list>
#include "Entity.h"
#include "CameraOrganizer.h"
#include "Bullet.h"
#include "Reticle.h"
#include "../../RailPath.h"

class EnemyManager;
class BaseEnemy;

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
	void SetRail(const RailPath* rail) { railPath_ = rail; }
	void SetEnemyManager(EnemyManager* enemyManager) { enemyManager_ = enemyManager; }
	
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
	const RailPath* railPath_ = nullptr;
	EnemyManager* enemyManager_ = nullptr;
	BaseEnemy* lockedEnemy_ = nullptr;
	Vector3 localTranslate_ = { 0.0f, 0.0f, 0.0f };

	float lockRadius_ = 40.0f; // スクリーン上のロックオン許容ピクセル半径
};