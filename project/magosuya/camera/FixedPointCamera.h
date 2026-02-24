#pragma once
#include "CameraComponent.h"
#include "InputManager.h"

class FixedPointCamera : public CameraComponent {
public:
	FixedPointCamera ();
	~FixedPointCamera () override;

	void Initialize (const EulerTransform& transform) override;

	void Update () override;

	void ImGui () override;

private:

	//ImGui識別用変数
	static inline int instanceNum_ = 0;
};