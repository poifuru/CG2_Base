#pragma once
#include <memory>
#include "MaterialData.h"
#include "StructuredBuffer.h"
#include "struct.h"

namespace MyEngine::LowLevel {
	class GraphicsDevice;
}
class DescriptorHeapManager;

class Material {
public:
	Material() = default;
	~Material() = default;
	// デバイスやヒープを用いて初期化
	void Initialize(MyEngine::LowLevel::GraphicsDevice* device, DescriptorHeapManager* heapManager);
	// シェーダーやテクスチャの設定
	void SetShader(uint32_t vsID, uint32_t psID) { vsID_ = vsID; psID_ = psID; }
	void SetTextureIndex(uint32_t textureIndex) { textureIndex_ = textureIndex; }

	// マテリアルパラメータの設定 (変更されたらフラグを立てる)
	void SetColor(const Vector4& color) { data_.color = color; isDirty_ = true; }

	// UV個別設定用
	void SetUvScale(const Vector3& scale) { uvTransform_.scale = scale; isDirty_ = true; }
	void SetUvRotate(const Vector3& rotate) { uvTransform_.rotate = rotate; isDirty_ = true; }
	void SetUvTranslate(const Vector3& translate) { uvTransform_.translate = translate; isDirty_ = true; }
	// まとめて設定したい時用
	void SetUvTransform(const EulerTransform& transform) { uvTransform_ = transform; isDirty_ = true; }

	void SetRoughness(float r) { data_.roughness = r; isDirty_ = true; }
	void SetMetallic(float m) { data_.metallic = m; isDirty_ = true; }
	void SetEnvironmentCoefficient(float e) { data_.environmentCoefficient = e; isDirty_ = true; }
	void SetEnableLighting(BOOL flag) { data_.enableLighting = flag; isDirty_ = true; }
	// 必要に応じてGPUにデータをアップロードする
	void Update();

	// 描画時などに使うゲッター群
	uint32_t GetVsID() const { return vsID_; }
	uint32_t GetPsID() const { return psID_; }
	uint32_t GetTextureIndex() const { return textureIndex_; }
	uint32_t GetDescriptorIndex() const { return buffer_ ? buffer_->GetDescriptorIndex() : 0; }

	// ImGuiでのパラメータ変更用
	const MaterialData& GetMaterialData() const { return data_; }
	const EulerTransform& GetUvTransform() const { return uvTransform_; }
private:
	// CPU側のパラメータ
	MaterialData data_{};
	EulerTransform uvTransform_{};
	uint32_t vsID_ = 0;
	uint32_t psID_ = 0;
	uint32_t textureIndex_ = 0;

	// GPU側のバッファ (StructuredBufferをマテリアル自身に持たせる)
	std::unique_ptr<StructuredBuffer<MaterialData>> buffer_ = nullptr;

	// CPUデータが書き換わったかどうかの更新フラグ (無駄な転送を避ける)
	bool isDirty_ = true;
};