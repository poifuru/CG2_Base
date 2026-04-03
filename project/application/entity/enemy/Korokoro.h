#pragma once
#include "BaseEnemy.h"

class Korokoro : public BaseEnemy {
public:
	Korokoro(DxCommon* dxCommon, LightManager* light, MapChip* mapchip);
	~Korokoro();

	void Initialize() override;
	void Update() override;
	void ImGui();
};

