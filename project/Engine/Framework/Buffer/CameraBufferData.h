#pragma once
#include "ConstantBuffer.h"
#include "struct.h"

// GPUへの転送用カメラデータ構造体
struct CameraBufferData {
	Vector3 worldPosition;
	float padding; // HLSLのアライメント調整用
};

struct CameraResource {
	ConstantBuffer<CameraBufferData> buffer;

	// メソッド
	void Initialize(ID3D12Device* device) { buffer.Initialize(device); }
	void Update(const CameraBufferData& data) { buffer.Update(data); }
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const { return buffer.GetGPUVirtualAddress(); }
};
