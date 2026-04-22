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

private:

};