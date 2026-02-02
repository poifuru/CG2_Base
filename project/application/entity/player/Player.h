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

	//武器
	std::unique_ptr<Weapon> weapon_ = nullptr;

	CameraOrganizer* camera_ = nullptr;
	InputManager* input_ = nullptr;
	LightManager* light_ = nullptr;
	MapChip* mapchip_ = nullptr;
};