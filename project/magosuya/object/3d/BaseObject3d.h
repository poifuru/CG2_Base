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

	virtual void ImGui(const std::string& label);

	// PSO周りの設定
	void SetRenderType(RenderType type);
	void SetBlendMode(BlendModeType type);

	// === 共通のアクセッサ（ゲッター・セッター） ===
	void SetScale(const Vector3& scale) { transform_.scale = scale; }
	void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { transform_.translate = translate; }
	void SetColor(const Vector4& color) { materialData_.color = color; }
	void SetRoughness(float r) { materialData_.roughness = r; }
	void SetMetallic(float m) { materialData_.metallic = m; }
	void SetLightingModel(BOOL flag) { materialData_.enableLighting = flag; }
	void SetTexture(const std::string& filePath);

protected:
	virtual Matrix4x4 CalculateWorldMatrix();

protected:
	BaseObject3d() = default;

	// BaseObject3d全体で共有するID用の数値
	static inline uint32_t instanceID_ = 0;

	// CPUデータ
	EulerTransform transform_{};
	EulerTransform uvTransform_{};
	MaterialData materialData_{};
	TransformMatrixData transformMatrixData_{};
	MaterialTex texInfo_{};

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