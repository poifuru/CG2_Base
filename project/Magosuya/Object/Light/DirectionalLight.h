#pragma once
#include "BaseLight.h"
#include "struct.h"

class DirectionalLight : public BaseLight {
public:
	DirectionalLight();
	~DirectionalLight() override;
	void Initialize() override;
	void Update() override;
	void ImGui(int index);

	//アクセッサ
	const Vector3& GetDirection() const { return direction_; }
	void SetDirection(const Vector3& dir) { direction_ = dir; }
	
private:
	//向き
	Vector3 direction_;
};