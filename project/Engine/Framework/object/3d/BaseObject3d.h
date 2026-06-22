#pragma once
#include <string>
#include <d3d12.h>
#include <memory>
#include "struct.h"
#include "TransformMatrixData.h"
#include "Material.h"
#include "CameraComponent.h"
#include "StructuredBuffer.h" // 構造化バッファをインクルード

class BaseObject3d {
public:
	BaseObject3d();
	virtual ~BaseObject3d() = default;

	void Initialize();

	virtual void Update(CameraData* cameraData);

	virtual void Draw(class RenderSystem* renderSystem) = 0;

	virtual void ImGui(const std::string& label);

	// === 共通のアクセッサ ===
	void SetParentTransform(const EulerTransform* parentTransform) { parentTransform_ = parentTransform; }
	void SetScale(const Vector3& scale) { transform_.scale = scale; }
	void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { transform_.translate = translate; }

	// マテリアルをセットする
	void SetMaterial(const std::shared_ptr<Material>& material) { material_ = material; }
	std::shared_ptr<Material> GetMaterial() const { return material_; }

	// 外部（ファクトリ）から初期化済みの共通バッファを受け取る
	void SetTransformBuffer(std::unique_ptr<TransformMatrixResource>&& transformBuffer) {
		transformBuffer_ = std::move(transformBuffer);
	}

	D3D12_GPU_VIRTUAL_ADDRESS GetTransformGPUAddress() const { return transformBuffer_ ? transformBuffer_->GetGPUVirtualAddress() : 0; }
	uint32_t GetMaterialDescriptorIndex() const { return material_ ? material_->GetDescriptorIndex() : 0; } // バインドレス用インデックスゲッター

protected:
	virtual Matrix4x4 CalculateWorldMatrix();

protected:
	static inline uint32_t instanceID_ = 0;

	// CPUデータ
	EulerTransform transform_{};
	const EulerTransform* parentTransform_ = nullptr;
	TransformMatrixData transformMatrixData_{};

	std::unique_ptr<TransformMatrixResource> transformBuffer_ = nullptr;
	std::shared_ptr<Material> material_ = nullptr;

	uint8_t layer_ = 1;
};
