#pragma once
#include "Entity.h"
#include "CameraOrganizer.h"
#include "Weapon.h"

class Player : public Entity {
public:		//メンバ関数
	Player(DxCommon* dxCommon, CameraOrganizer* camera, InputManager* input, LightManager* light, MapChip* mapchip);
	~Player();

	void Initialize() override;
	void Update() override;
	void Draw() override;
	void ImGui();

	//ゲッター
	Transform GetTransform() { return transform_; }
	Weapon* GetWeapon() { return weapon_.get(); }
	void SetVelocity(const Vector3& velocity) { velocity_ = velocity; }
	void ResetDoubleJump() { isDoubleJump_ = false; }
	float GetUpDownDir() { return updownDirection_; }
	AABB GetAABB() { return aabb_; }
	void OnDamageFromEnemy();

private:	//プレイヤーだけのメソッド
	void Input();
	void FreeFall();
	void WallKickTimer();
	void ProcessDash();
	//Entityクラスに合わせる状態変化
	void EntityState();

private:
	//プレイヤー専用パラメータ
	bool isDoubleJump_ = false;
	int wallKickTimer_ = 0; // 壁キック後の入力禁止タイマー
	bool isDashing_ = false;     // ダッシュ中か
	int dashTimer_ = 0;          // ダッシュ持続タイマー
	int dashCooldown_ = 0;       // クールタイム
	float dashDirection_ = 1.0f; // ダッシュする方向（1.0f か -1.0f）
	float faceDirection_ = 1.0f; // 1.0fなら右、-1.0fなら左
	float updownDirection_ = 0.0f;

	//ダメージ
	int damageTimer_ = 0; // 連続ダメージ防止用タイマー

	//スプライト
	int shakeTimer_ = 0;      // シェイク演出用のタイマー
	int lastHp_ = 0;          // 前フレームのHPを覚えておく用

	//武器
	std::unique_ptr<Weapon> weapon_ = nullptr;
	std::unique_ptr<Sprite> life_[3];

	CameraOrganizer* camera_ = nullptr;
	InputManager* input_ = nullptr;
	LightManager* light_ = nullptr;
	MapChip* mapchip_ = nullptr;
};