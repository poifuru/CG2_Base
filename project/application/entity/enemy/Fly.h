#pragma once
#include "BaseEnemy.h"

class Fly : public BaseEnemy {
public:
	// 親クラスと同じ引数のコンストラクタを用意する
	Fly(DxCommon* dxCommon, LightManager* light, MapChip* mapchip);

	void Initialize() override;
	void Update() override;

private:
	float floatingTimer_ = 0.0f; // ふよふよ動くためのタイマー
	float baseHeight_ = 0.0f;    // 基準となる高さ
};

