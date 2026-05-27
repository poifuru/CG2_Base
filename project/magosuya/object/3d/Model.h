#pragma once
#include "BaseObject3d.h"
#include "MeshData.h"
#include "Animation.h"

class Animator;

class Model : public BaseObject3d {
public:
	Model(DxCommon* dxCommon);
	~Model() override;

	// アセットはファクトリーから直接流してもらう
	void Initialize(ModelData* modelData);

	void Update(CameraData* cameraData) override;

	void Draw() override;

	void ImGui(const std::string& label);

	// モデルデータを外に渡せるように
	ModelData* GetModelData() const { return modelData_; }

	// アニメーションを切り替えるセッター（ポインタで直接受け取る）
	void SetAnimator(Animator* animator);
	void SetModelData(ModelData* modelData) { modelData_ = modelData; }

private:	// 内部メソッド
	Matrix4x4 CalculateWorldMatrix() override;

private:
	// モデルのデータ
	ModelData* modelData_ = nullptr;

	// アニメーション再生用のデータ
	Animator* animator_ = nullptr;
};