#pragma once
#include <string>
#include <d3d12.h>
#include "struct.h"             // Matrix4x4, Vector4 など
#include "Buffer.h"             // VertexBuffer, IndexBuffer, ConstantBuffer
#include "TransformMatrixData.h" // TransformMatrixData
#include "MaterialData.h"        // MaterialData
#include "RenderCommand.h"       // PSODescriptor
#include "CameraComponent.h"

class DxCommon;

struct SkyboxVertex {
	Vector4 position;
};

class Skybox {
public:
	Skybox(DxCommon* dxCommon);
	~Skybox() = default;

	void Initialize(const std::string& textureTag);

	void Update(CameraData* data);

	void Draw();

private:
	// PSOの設定
	PSODescriptor psoDesc_{};

	// CPUデータ（きっちり隔離）
	TransformMatrixData cpuTransformData_{};
	MaterialData cpuMaterialData_{};

	// GPUリソース
	VertexBuffer<SkyboxVertex> vertexBuffer_;
	IndexBuffer<uint32_t> indexBuffer_;
	ConstantBuffer<TransformMatrixData> matrixBuffer_;
	ConstantBuffer<MaterialData> materialBuffer_;

	// テクスチャを引っ張ってくるための識別タグ
	std::string tag_;

	// 借りるポインタ
	DxCommon* dxCommon_ = nullptr;
	ID3D12Device* device_ = nullptr;
	ID3D12GraphicsCommandList* commandList_ = nullptr;
};