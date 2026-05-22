#pragma once
#include "BaseObject3d.h"
#include "MeshData.h"
#include "Animation.h"

class Model : public BaseObject3d {
public:
	Model(DxCommon* dxCommon);
	~Model() override;

	// アセットはファクトリーから直接流してもらう
	void Initialize(ModelData* modelData, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle);

	void Update(CameraData* cameraData) override;

	void Draw() override;

	void ImGui(const std::string& label);

	// アニメーションを切り替えるセッター（ポインタで直接受け取る）
	void SetAnimation(Animation* animation) { animation_ = animation; animationTime_ = 0.0f; }
	void SetModelData(ModelData* modelData) { modelData_ = modelData; }

private:	// 内部メソッド
	Matrix4x4 CalculateWorldMatrix() override;
	Matrix4x4 AnimationUpdate();

private:
	// モデルのデータ
	ModelData* modelData_ = nullptr;

	// アニメーション再生用のデータ
	Animation* animation_ = nullptr;
	float animationTime_ = 0.0f;
};