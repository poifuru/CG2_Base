#pragma once
#include "BaseLight.h"

class RectLight : public BaseLight {
public:
	RectLight();
	~RectLight() override;
	void Initialize() override;
	void Update() override;
	void ImGui(int index);

	const Vector3& GetPosition() { return position_; }
	const Vector3& GetDirection() { return direction_; }
	const Vector2& GetSize() { return size_; }
	const Vector3& GetRight() { return right_; }
	const Vector3& GetUp() { return up_; }
	const float& GetDecay() { return decay_; }

private:
	Vector3 position_;
	Vector3 direction_;
	Vector2 size_;
	Vector3 right_;
	Vector3 up_;
	float decay_;
};

