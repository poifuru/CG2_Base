#pragma once
#include <memory>
#include "MaterialData.h"
#include "StructuredBuffer.h"
#include "InputLayoutManager.h"
#include "BlendModeManager.h"

namespace MyEngine::LowLevel {
	class GraphicsDevice;
	class DescriptorHeapManager;
}

namespace MyEngine::Rendering {
	class RootSignatureManager;
	class PSOManager;
	class ShaderManager;
}

class Material {
public:
	Material() = default;
	~Material() = default;

	// デバイスやヒープを用いて初期化
	void Initialize(
		MyEngine::LowLevel::GraphicsDevice* device,
		MyEngine::LowLevel::DescriptorHeapManager* heapManager,
		MyEngine::Rendering::RootSignatureManager* rootSigManager, 
		MyEngine::Rendering::PSOManager* psoManager,
		MyEngine::Rendering::ShaderManager* shaderManager,
		MyEngine::Rendering::InputLayoutManager* inputLayoutManager,
		MyEngine::Rendering::BlendModeManager* blendModeManager
	);

	void Update();
	
	// シェーダーやテクスチャの設定
	void SetShader(uint32_t vsID, uint32_t psID);
	void SetBlendMode(MyEngine::Rendering::BlendModeType blenMode);
	void SetDepthEnable(bool enable);
	void SetDoubleSided(bool doubleSided);
	void SetInputLayout(MyEngine::Rendering::InputLayoutType layoutType) { inputLayout_ = layoutType; UpdatePSO(); }
	void SetDepthWrite(bool writeEnable) { depthWriteEnable_ = writeEnable; UpdatePSO(); }
	void SetTextureIndex(uint32_t textureIndex) { textureIndex_ = textureIndex; }

	// アクセッサ
	ID3D12PipelineState* GetPSO() const { return pso_; }
	uint32_t GetTextureIndex() const { return textureIndex_; }
	uint32_t GetDescriptorIndex() const { return buffer_ ? buffer_->GetDescriptorIndex() : 0; } // 定数バッファのヒープインデックス

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

	// ImGuiでのパラメータ変更用
	const MaterialData& GetMaterialData() const { return data_; }
	const EulerTransform& GetUvTransform() const { return uvTransform_; }

private:
	void UpdatePSO();

private:
	// 依存関係（PSO生成に必要）
	MyEngine::LowLevel::GraphicsDevice* device_ = nullptr;
	MyEngine::Rendering::RootSignatureManager* rootSigManager_ = nullptr;
	MyEngine::Rendering::PSOManager* psoManager_ = nullptr;
	MyEngine::Rendering::ShaderManager* shaderManager_ = nullptr;
	MyEngine::Rendering::InputLayoutManager* inputLayoutManager_ = nullptr;
	MyEngine::Rendering::BlendModeManager* blendModeManager_ = nullptr;

	// 高レベルな描画パラメータ
	uint32_t vsID_ = 0;
	uint32_t psID_ = 0;

	MyEngine::Rendering::BlendModeType blendMode_ = MyEngine::Rendering::BlendModeType::Opaque;
	MyEngine::Rendering::InputLayoutType inputLayout_ = MyEngine::Rendering::InputLayoutType::Standard3D;
	bool depthWriteEnable_ = true;

	bool isDepthEnable_ = true;
	bool isDoubleSided_ = false;

	uint32_t textureIndex_ = 0;
	// 生成されたパイプラインステートへのポインタを直接保持する
	ID3D12PipelineState* pso_ = nullptr;

	// GPU側のバッファ (StructuredBufferをマテリアル自身に持たせる)
	std::unique_ptr<StructuredBuffer<MaterialData>> buffer_ = nullptr;
	// CPU側のパラメータ
	MaterialData data_{};
	EulerTransform uvTransform_{};
	// CPUデータが書き換わったかどうかの更新フラグ (無駄な転送を避ける)
	bool isDirty_ = true;
};