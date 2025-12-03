#pragma once
#include "CameraComponent.h"

class FollowCamera : public CameraComponent {
public:
	FollowCamera ();
	~FollowCamera () override;

	void Initialize (const Transform& transform) override;

	void SetTarget (const Vector3* target);

	void Update () override;

	void ImGui () override;

	const Vector3& GetOffset () { return offset_; }
	void SetOffset (const Vector3& offset) { offset_ = offset; }

private:
	//追従する対象のポインタ
	Vector3* target_ = nullptr;

	//対象とカメラの距離を一定に保つためのオフセット
	Vector3 offset_ = {};

	//ImGui識別用変数
	static inline int instanceNum_ = 0;
};