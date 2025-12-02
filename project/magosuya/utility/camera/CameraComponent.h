#pragma once
#include "struct.h"

class CameraComponent {
public:
	Matrix4x4* GetViewMat ();

protected:
	Transform transform_;
	Matrix4x4 world_;
	Matrix4x4 view_;
	Matrix4x4 projection_;
};