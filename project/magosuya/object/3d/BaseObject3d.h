#pragma once
#include <string>
#include <d3d12.h>
#include <memory>
#include "struct.h"
#include "TransformMatrixData.h"
#include "MaterialData.h"
#include "RenderSystem.h"
#include "CameraComponent.h"

class DxCommon;
class LightManager;

class BaseObject3d {
public:
	BaseObject3d(DxCommon* dxCommon, LightManager* lightManager);
	virtual ~BaseObject3d() = default;

	void Initialize();

	virtual void Update(CameraData* cameraData);

	// 自身はメッシュを持たないので純粋仮想関数
	virtual void Draw() = 0;

	// === 共通のアクセッサ（ゲッター・セッター） ===
	void SetColor(const Vector4& color) { materialData_.color = color; }
	void SetRoughness(float r) { materialData_.roughness = r; }
	void SetMetallic(float m) { materialData_.metallic = m; }
	void SetLightingModel(BOOL flag) { materialData_.enableLighting = flag; }
	void SetTexture(D3D12_GPU_DESCRIPTOR_HANDLE handle) { textureHandle_ = handle; }

protected:
	BaseObject3d() = default;

	// CPUデータ
	EulerTransform transform_{};
	EulerTransform uvTransform_{};
	MaterialData materialData_{};
	TransformMatrixData transformData_{};

	// トランスフォームとマテリアル（すべての3D物が個別に持つバッファ）
	std::unique_ptr<TransformMatrixResource> transformResource_ = nullptr;;
	std::unique_ptr<MaterialResource> materialResource_;
	D3D12_GPU_DESCRIPTOR_HANDLE textureHandle_{};

	// パイプライン設定（子クラスの Initialize で具体的なIDを詰めさせる）
	uint32_t rootSignatureID_ = 0;
	PSODescriptor psoDesc_{};

	DxCommon* dxCommon_ = nullptr;
	LightManager* lightManager_ = nullptr;
};