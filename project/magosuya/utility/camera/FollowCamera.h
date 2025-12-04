#pragma once
#include "CameraComponent.h"

class FollowCamera : public CameraComponent {
public:
	FollowCamera ();
	~FollowCamera () override;

	void Initialize (const Transform& transform) override;

	void SetTarget (const Transform* target);

	void Update () override;

	void ImGui () override;

	const Vector3& GetOffset () { return offset_; }
	void SetOffset (const Vector3& offset) { offset_ = offset; }
	const float& GetSmoothness () { return smoothness_; }
	void SetSmoothness (const float& smoothness) { smoothness_ = smoothness; }

private:
	//追従する対象のポインタ
	Transform* target_ = nullptr;

	//対象とカメラの距離を一定に保つためのオフセット
	Vector3 offset_ = {};

	//カメラの追従感度
	float smoothness_ = 0.0f;

	//ImGui識別用変数
	static inline int instanceNum_ = 0;
};