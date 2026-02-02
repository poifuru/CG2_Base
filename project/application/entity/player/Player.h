#pragma once
#include "Entity.h"
#include "CameraOrganizer.h"

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
	//Entityクラスに合わせる状態変化
	void EntityState();

private:
	//プレイヤー専用パラメータ
	bool isDoubleJump_ = false;
	int wallKickTimer_ = 0; // 壁キック後の入力禁止タイマー

	CameraOrganizer* camera_ = nullptr;
	InputManager* input_ = nullptr;
	LightManager* light_ = nullptr;
	MapChip* mapchip_ = nullptr;
};