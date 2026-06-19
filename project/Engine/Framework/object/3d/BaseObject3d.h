#pragma once
#include <string>
#include <d3d12.h>
#include <memory>
#include "struct.h"
#include "TransformMatrixData.h"
#include "MaterialData.h"
#include "CameraComponent.h"
#include "StructuredBuffer.h" // 構造化バッファをインクルード

class BaseObject3d {
public:
	BaseObject3d();
	virtual ~BaseObject3d() = default;

	void Initialize();

	virtual void Update(CameraData* cameraData);

	virtual void Draw(class RenderSystem* renderSystem) = 0; // 引数に RenderSystem を渡す

	virtual void ImGui(const std::string& label);

	// === 共通のアクセッサ ===
	void SetScale(const Vector3& scale) { transform_.scale = scale; }
	void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { transform_.translate = translate; }
	void SetColor(const Vector4& color) { materialData_.color = color; }
	void SetRoughness(float r) { materialData_.roughness = r; }
	void SetMetallic(float m) { materialData_.metallic = m; }
	void SetLightingModel(BOOL flag) { materialData_.enableLighting = flag; }

	// 外部（ファクトリ）から初期化済みの共通バッファを受け取る
	void SetCommonBuffers(
		std::unique_ptr<TransformMatrixResource>&& transformBuffer,
		std::unique_ptr<StructuredBuffer<MaterialData>>&& materialBuffer // StructuredBufferへ変更
	) {
		transformBuffer_ = std::move(transformBuffer);
		materialBuffer_ = std::move(materialBuffer);
	}

	D3D12_GPU_VIRTUAL_ADDRESS GetTransformGPUAddress() const { return transformBuffer_ ? transformBuffer_->GetGPUVirtualAddress() : 0; }
	uint32_t GetMaterialDescriptorIndex() const { return materialBuffer_ ? materialBuffer_->GetDescriptorIndex() : 0; } // バインドレス用インデックスゲッター
	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureHandle() const { return textureHandle_; }

protected:
	virtual Matrix4x4 CalculateWorldMatrix();

protected:
	static inline uint32_t instanceID_ = 0;

	// CPUデータ
	EulerTransform transform_{};
	EulerTransform uvTransform_{};
	MaterialData materialData_{};
	TransformMatrixData transformMatrixData_{};

	std::unique_ptr<TransformMatrixResource> transformBuffer_ = nullptr;
	std::unique_ptr<StructuredBuffer<MaterialData>> materialBuffer_ = nullptr; // StructuredBufferへ変更
	D3D12_GPU_DESCRIPTOR_HANDLE textureHandle_{};

	uint8_t layer_ = 1;
};
