#pragma once
#include "Entity.h"
#include "CameraOrganizer.h"

class Reticle : public Entity {
public:
	Reticle(DxCommon* dxCommon, CameraOrganizer* camera, InputManager* input, LightManager* light);

	~Reticle();

	void Initialize() override;

	void Update() override;

	void Draw() override;

	void ImGui();

	// アクセッサ
	void SetPlayerPos(const Vector3& pos) { playerPos_ = pos; }

private:
	void Input();

private:
	Vector3 playerPos_{};
	Vector3 positionOfset_{};
	float speed_ = 0.0f;

	InputManager* input_ = nullptr;
	CameraOrganizer* camera_ = nullptr;
};