#pragma once
#include <string>
#include <d3d12.h>
#include <memory>
#include "struct.h"
#include "TransformMatrixData.h"
#include "MaterialData.h"
#include "RenderSystem.h"
#include "RenderCommand.h"
#include "CameraComponent.h"
#include "TextureManager.h"

class DxCommon;
class LightManager;

class BaseObject3d {
public:
	BaseObject3d(DxCommon* dxCommon);
	virtual ~BaseObject3d() = default;

	void Initialize();

	virtual void Update(CameraData* cameraData);

	// 自身はメッシュを持たないので純粋仮想関数
	// システム側に共通バッファがあるので、配列3番からセットすること
	virtual void Draw() = 0;

	// PSO周りの設定
	void SetRenderType(RenderType type);
	void SetBlendMode(BlendModeType type);

	// === 共通のアクセッサ（ゲッター・セッター） ===
	void SetColor(const Vector4& color) { materialData_.color = color; }
	void SetRoughness(float r) { materialData_.roughness = r; }
	void SetMetallic(float m) { materialData_.metallic = m; }
	void SetLightingModel(BOOL flag) { materialData_.enableLighting = flag; }
	void SetTexture(const std::string& ID) { textureHandle_ = TextureManager::GetInstance()->GetTextureHandle(ID); }

protected:
	BaseObject3d() = default;

	// CPUデータ
	EulerTransform transform_{};
	EulerTransform uvTransform_{};
	MaterialData materialData_{};
	TransformMatrixData transformMatrixData_{};

	// トランスフォームとマテリアル（すべての3D物が個別に持つバッファ）
	std::unique_ptr<TransformMatrixResource> transformBuffer_ = nullptr;;
	std::unique_ptr<MaterialResource> materialBuffer_ = nullptr;
	D3D12_GPU_DESCRIPTOR_HANDLE textureHandle_{};

	// パイプライン設定（子クラスの Initialize で具体的なIDを詰めさせる）
	PSODescriptor psoDesc_{};
	uint8_t layer_ = 1;
	RenderType renderType_ = RenderType::Standard;

	DxCommon* dxCommon_ = nullptr;
};