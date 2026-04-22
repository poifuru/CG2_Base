#pragma once
#include "Entity.h"
#include "CameraOrganizer.h"

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

private:
	void Move();
	void Life();

private:
	bool isActive_ = false;
	float activeTimer_ = 0.0f;

	CameraOrganizer* camera_ = nullptr;
};