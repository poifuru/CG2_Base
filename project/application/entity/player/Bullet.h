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

private:

};