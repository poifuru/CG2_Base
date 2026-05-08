#pragma once
#include "BaseEnemy.h"
class SmallFish : public BaseEnemy {
public:
	SmallFish(DxCommon* dxCommon, LightManager* light, CameraOrganizer* camera);
	~SmallFish();

	void Initialize() override;

	void Update() override;

	void Draw() override;

	void ImGui();

	// アクセッサ
	void SetPostion(const Vector3& pos) { transform_.translate = pos; }
};