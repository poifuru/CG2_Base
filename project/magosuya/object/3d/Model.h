#pragma once
#include "BaseObject3d.h"
#include "MeshData.h"
#include "Animation.h"

class Model : public BaseObject3d {
public:
	Model(DxCommon* dxCommon, LightManager* lightManager);
	~Model() override;

	// アセットはファクトリーから直接流してもらう
	void Initialize(ModelData* modelData, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle);

	void Update(CameraData* cameraData) override;

	void Draw() override;

	// アニメーションを切り替えるセッター（ポインタで直接受け取る）
	void SetAnimation(Animation* animation) { animation_ = animation; animationTime_ = 0.0f; }
	void SetModelData(ModelData* modelData) { modelData_ = modelData; }

private:
	// モデルのデータ
	ModelData* modelData_ = nullptr;

	// アニメーション再生用のデータ
	Animation* animation_ = nullptr;
	float animationTime_ = 0.0f;
};