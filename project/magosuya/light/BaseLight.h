#pragma once
#include "struct.h"

class BaseLight {
public:
	//メンバ関数(純粋仮想)
	virtual ~BaseLight() = default;
	virtual void Initialize() = 0;
	virtual void Update() = 0;

	//アクセッサー
	const Vector4& GetColor() const { return color_; }
	const float& GetIntensity() const { return intensity_; }

protected:
	Vector4 color_;		//ライトの色
	float intensity_;	//輝度
};