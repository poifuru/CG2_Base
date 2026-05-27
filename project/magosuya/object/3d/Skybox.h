#pragma once
#include <string>
#include <d3d12.h>
#include "struct.h"             // Matrix4x4, Vector4 など
#include "Buffer.h"             // VertexBuffer, IndexBuffer, ConstantBuffer
#include "TransformMatrixData.h" // TransformMatrixData
#include "MaterialData.h"        // MaterialData
#include "RenderCommand.h"       // PSODescriptor
#include "CameraComponent.h"
#include "BaseObject3d.h"

struct SkyboxVertex {
	Vector4 position;
};

class Skybox : public BaseObject3d {
public:
	Skybox(DxCommon* dxCommon);
	~Skybox() = default;

	void Initialize(const std::string& filePath);

	void Update(CameraData* data);

	void Draw();

	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureHandle() { return TextureManager::GetInstance()->GetTextureHandle(texInfo_.index); }

private:
	// 頂点バッファ
	std::unique_ptr<VertexBuffer<SkyboxVertex>> vertexBuffer_ = nullptr;
	// インデックスバッファ
	std::unique_ptr<IndexBuffer<uint32_t>> indexBuffer_ = nullptr;

	// テクスチャを引っ張ってくるための識別タグ
	std::string tag_;
};