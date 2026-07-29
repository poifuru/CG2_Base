#pragma once
#include "CameraOrganizer.h"

class BaseCamera {
public:
	virtual ~BaseCamera () = default;

	virtual void Initialize (const EulerTransform& transform) = 0;
	virtual void Update () = 0;
	virtual void ImGui () = 0;

	CameraData& GetCameraData() { return camera_; }
	const EulerTransform& GetEulerTransform () { return camera_.transform; }
	
	Vector3 GetScale () { return camera_.transform.scale; }
	void SetScale (const Vector3& scale) { camera_.transform.scale = scale; }
	Vector3 GetRotate () { return camera_.transform.rotate; }
	void SetRotate (const Vector3& rotate) { camera_.transform.rotate = rotate; }
	Vector3 GetTranslate () { return camera_.transform.translate; }
	void SetTranslate (const Vector3& translate) { camera_.transform.translate = translate; }
	
	const Matrix4x4& GetWorldMat () { return camera_.world; }
	const Matrix4x4& GetViewMat () { return camera_.view; };
	const Matrix4x4& GetProjMat () { return camera_.proj; }
	const Matrix4x4& GetVPMat () { return camera_.vp; }
	
	const float& GetNear() { return camera_.nearClip; }
	const float& GetFar() { return camera_.farClip; }

protected:
	CameraData camera_ = {};
};