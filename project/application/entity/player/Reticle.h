#pragma once
#include "Entity.h"
#include "CameraOrganizer.h"
#include "../../RailPath.h"

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
	void SetPlayerLocalPos(const Vector3& pos) { playerLocalPos_ = pos; }
	void SetRail(const RailPath* rail) { railPath_ = rail; }
	void SetLockOn(bool lock);
	Vector3 GetPosition() { return transform_.translate; }

private:
	void Input();

private:
	Vector3 playerPos_{};
	Vector3 playerLocalPos_{};
	Vector3 localTranslate_{};
	const RailPath* railPath_ = nullptr;
	bool isLockOn_ = false;
	Vector3 positionOfset_{};
	float speed_ = 0.0f;

	InputManager* input_ = nullptr;
};