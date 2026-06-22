#pragma once
#include <string>
#include "ConstantBuffer.h"
#include "struct.h"

// マテリアルデータ
struct MaterialData {
	Vector4 color;
	Matrix4x4 uvTransform;
	float roughness; // 粗さ
	float metallic; // 金属度
	float environmentCoefficient;	// 環境係数
	BOOL enableLighting;
};

// テクスチャのファイルパス保存用
struct MaterialTex {
	int index;
	std::string filePath;
};

struct MaterialResource {
	ConstantBuffer<MaterialData> buffer;

	// メソッド
	void Initialize(ID3D12Device* device) { buffer.Initialize(device); }
	void Update(const MaterialData& data) { buffer.Update(data); }
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const { return buffer.GetGPUVirtualAddress(); }
};