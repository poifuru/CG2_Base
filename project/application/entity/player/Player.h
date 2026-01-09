#pragma once
#include "Entity.h"
#include "CameraOrganizer.h"

//旋回時間(秒)
static inline const float kTimeTurn = 0.16f;

class Player : public Entity {
public:		//メンバ関数
	enum Direction {
		Left,
		Right
	};

	Player(DxCommon* dxCommon, CameraOrganizer* camera);
	~Player();

	void Initialize() override;
	void Update() override;
	void Draw() override;
	void IsHit();
	void SetLight(ID3D12Resource* light) { light_ = light; }

private:
	void move();
	void Jump();
	void Avoid();
	void Attack();
	void WeaponTransform();
	void AABBPos();
	void Invincible();
	void TurnControll();

public:
	//アクセッサ
	Transform GetTransform() { return model_->GetTransform(); }
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

	//旋回開始時の角度
	float turnFirstRotationY_ = 0.0f;
	//旋回タイマー
	float turnTimer_ = 0.0f;

	bool isUpperAttack_ = false; // 上攻撃かどうかを判定

	CameraOrganizer* camera_ = nullptr;
	ID3D12Resource* light_ = nullptr;
	InputManager* input_ = nullptr;
};