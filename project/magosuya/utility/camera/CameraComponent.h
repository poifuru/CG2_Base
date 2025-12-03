#pragma once
#include "struct.h"
#include "WindowsAPI.h"
#include "MathFunction.h"

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
	const Transform& GetTransform () { return transform_; }
	//scale設定
	void SetScale (const Vector3& scale) { transform_.scale = scale; }
	//rotate設定
	void SetRotate (const Vector3& rotate) { transform_.rotate = rotate; }
	//translate設定
	void SetTranslate (const Vector3& translate) { transform_.translate = translate; }
	//world行列取得
	const Matrix4x4& GetWorldMat () { return world_; }
	//view行列取得
	const Matrix4x4& GetViewMat () { return view_; };
	//projection行列取得
	const Matrix4x4& GetProjMat () { return proj_; }
	//vp行列取得
	const Matrix4x4& GetVPMat () { return Multiply (view_, proj_); }

protected:
	Transform transform_;
	Matrix4x4 world_;
	Matrix4x4 view_;
	Matrix4x4 proj_;

	//ポインタを借りる
	WindowsAPI* winAPI_ = nullptr;
};