#include "BaseObject3d.h"
#include "MathFunction.h"

BaseObject3d::BaseObject3d(DxCommon* dxCommon, LightManager* lightManager) {
	dxCommon_ = dxCommon;
	lightManager_ = lightManager;

	transformResource_ = std::make_unique<TransformMatrixResource>();
	materialResource_ = std::make_unique<MaterialResource>();
}

void BaseObject3d::Initialize() {
	// 共通バッファの生成
	transformResource_->Initialize(dxCommon_);
	materialResource_->Initialize(dxCommon_);

	// CPUデータの初期化
	// デフォルト値のセット
	transform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	uvTransform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	materialData_.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData_.enableLighting = TRUE;
	materialData_.roughness = 0.3f;
	materialData_.metallic = 0.5f;
	materialData_.environmentCoefficient = 0.3f;
}

void BaseObject3d::Update(CameraData* cameraData) {
	// === 行列の計算とGPUへの転送をここで一括処理 ===
	Matrix4x4 world = Math::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	transformData_.World = world;
	transformData_.WVP = world * cameraData->vp;
	transformData_.WorldInverseTranspose = Math::Inverse(Math::Transpose(world));

	materialData_.uvTransform = Math::MakeAffineMatrix(uvTransform_.scale, uvTransform_.rotate, uvTransform_.translate);

	// GPUにドン！
	transformResource_->Update(transformData_);
	materialResource_->Update(materialData_);
}

void BaseObject3d::Draw() {

}