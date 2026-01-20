#pragma once
#include "BaseLight.h"

class SpotLight : public BaseLight {
public:
	SpotLight();
	~SpotLight() override;
	void Initialize() override;
	void Update() override;
	void ImGui(int index);

	const Vector3& GetPosition() { return position_; }
	const Vector3& GetDirection() { return direction_; }
	const float& GetDistance() { return distance_; }
	const float& GetDecay() { return decay_; }
	const float& GetCosAngle() { return cosAngle_; }

private:
	Vector3 position_;
	Vector3 direction_;
	float distance_;	//ライトが届く最大	距離
	float decay_;	//減衰率
	float cosAngle_;	//スポットライトの余弦
};

