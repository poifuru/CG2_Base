#pragma once
#include "Entity.h"

enum Direction {
	Left,
	Right
};

class Player : public Entity {
public:		//メンバ関数
	Player(DxCommon* dxCommon);
	~Player();

	void Initialize();
	void Update();
	void Draw();
	void IsHit();

private:
	void move();
	void Jump();
	void Avoid();
	void Attack();
	void WeaponTransform();
	void AABBPos();
	void Invincible();

public:
	//アクセッサ
	int GetHp() { return hp_; }
	int SetHp(int hp) { return hp_ += hp; }
	AABB GetAABBModel() { return aabb_; }
	AABB GetAABBWeapon_() { return aabb_weapon_; }
	bool GetIsAttack() { return isAttack_; }
	bool GetAttackIsHit() { return attackIsHit_; }
	void SetAttackIsHit(bool flag) { attackIsHit_ = flag; }

private:	//メンバ変数
	std::unique_ptr<Model> weapon_ = nullptr;
	Transform transformWep_ = {};
	AABB aabb_weapon_ = {};
	Direction dir_ = Direction::Right;
	Direction attackDir_ = Direction::Right;
	int hp_ = 0;
	bool isInvincible_ = false;
	float invincibleTime_ = 0.0f;
	bool isAir_ = false;
	bool airJump_ = false;
	bool isAvoid_ = false;
	float avoidTimer_ = 0.0f;
	bool coolTime_ = false;
	float avoidCoolTime_ = 0.0f;
	bool isAttack_ = false;
	float attackTime_ = 0.0f;
	bool attackIsHit_ = false;
};