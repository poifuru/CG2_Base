#pragma once
#include "BaseEnemy.h"
class SmallFish : public BaseEnemy {
	SmallFish();
	~SmallFish();

	void Initialize() override;

	void Update() override;

	void Draw() override;

	void ImGui();

	// アクセッサ
	void SetPostion(const Vector3& pos) { transform_.translate = pos; }
};