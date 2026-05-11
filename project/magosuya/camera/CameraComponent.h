#pragma once
#include "struct.h"
#include "WindowsAPI.h"
#include "MathFunction.h"

struct CameraData {
	EulerTransform Transform;
	Matrix4x4 world;
	Matrix4x4 view;
	Matrix4x4 proj;
	Matrix4x4 vp;
};

class CameraComponent {
public:
	//デフォルトデストラクタ
	virtual ~CameraComponent () = default;

	//初期化処理 (純粋仮想関数)
	virtual void Initialize (const EulerTransform& Transform) = 0;
	//更新処理 (純粋仮想関数)
	virtual void Update () = 0;
	//ImGui (純粋仮想関数)
	virtual void ImGui () = 0;

	//CameraData取得
	CameraData& GetCameraData() { return camera_; }
	//Transform取得
	const EulerTransform& GetEulerTransform () { return camera_.Transform; }
	//scale設定
	Vector3 GetScale () { return camera_.Transform.scale; }
	void SetScale (const Vector3& scale) { camera_.Transform.scale = scale; }
	//rotate設定
	Vector3 GetRotate () { return camera_.Transform.rotate; }
	void SetRotate (const Vector3& rotate) { camera_.Transform.rotate = rotate; }
	//translate設定
	Vector3 GetTranslate () { return camera_.Transform.translate; }
	void SetTranslate (const Vector3& translate) { camera_.Transform.translate = translate; }
	//world行列取得
	const Matrix4x4& GetWorldMat () { return camera_.world; }
	//view行列取得
	const Matrix4x4& GetViewMat () { return camera_.view; };
	//projection行列取得
	const Matrix4x4& GetProjMat () { return camera_.proj; }
	//vp行列取得
	const Matrix4x4& GetVPMat () { return camera_.vp; }

protected:
	CameraData camera_ = {};
};