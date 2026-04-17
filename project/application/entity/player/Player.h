#pragma once
#include "Entity.h"
#include "CameraOrganizer.h"
#include "Weapon.h"

class Player : public Entity {
public:		//メンバ関数
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
	
private:	// プライベート関数
	void Input();
	void Move();

private:
	//プレイヤーのパラメータ
	float speed_ = 0.0f;

	CameraOrganizer* camera_ = nullptr;
	InputManager* input_ = nullptr;
	LightManager* light_ = nullptr;
};