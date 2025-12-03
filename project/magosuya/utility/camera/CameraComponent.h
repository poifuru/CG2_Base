#pragma once
#include "struct.h"
#include "WindowsAPI.h"
#include "MathFunction.h"

struct CameraData {
	Transform transform;
	Matrix4x4 world;
	Matrix4x4 view;
	Matrix4x4 proj;
};

class CameraComponent {
public:
	//デフォルトデストラクタ
	virtual ~CameraComponent () = default;

	//初期化処理 (純粋仮想関数)
	virtual void Initialize (const Transform& transform) = 0;
	//更新処理 (純粋仮想関数)
	virtual void Update () = 0;
	//ImGui (純粋仮想関数)
	virtual void ImGui () = 0;

	//transform取得
	const Transform& GetTransform () { return camera_->transform; }
	//scale設定
	void SetScale (const Vector3& scale) { camera_->transform.scale = scale; }
	//rotate設定
	void SetRotate (const Vector3& rotate) { camera_->transform.rotate = rotate; }
	//translate設定
	void SetTranslate (const Vector3& translate) { camera_->transform.translate = translate; }
	//world行列取得
	const Matrix4x4& GetWorldMat () { return camera_->world; }
	//view行列取得
	const Matrix4x4& GetViewMat () { return camera_->view; };
	//projection行列取得
	const Matrix4x4& GetProjMat () { return camera_->proj; }
	//vp行列取得
	const Matrix4x4& GetVPMat () { return Math::Multiply (camera_->view, camera_->proj); }

protected:
	CameraData* camera_;

	//ポインタを借りる
	WindowsAPI* winAPI_ = nullptr;
};