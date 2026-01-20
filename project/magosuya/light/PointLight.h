#pragma once
#include "BaseLight.h"

class PointLight : public BaseLight {
public:
	PointLight();
	~PointLight() override;
	void Initialize() override;
	void Update() override;
	void ImGui(int index);

	const Vector3& GetPosition() { return position_; }
	const float& GetRadius() { return radius_; }
	const float& GetDecay() { return decay_; }

private:
	Vector3 position_;
	float radius_;	//ライトが届く最大距離
	float decay_;	//減衰率
};