#pragma once
#include "Buffer.h"

// 行列データ
struct TransformMatrixData {
	Matrix4x4 World;
	Matrix4x4 WVP;
	Matrix4x4 WorldInverseTranspose;	// ライティングの時に正しい法線を計算する
};

struct TransformMatrixResource {
	ConstantBuffer<TransformMatrixData> buffer;

	// メソッド
	void Initialize(DxCommon* dxCommon) { buffer.Initialize(dxCommon); }
	void Update(const TransformMatrixData& data) { buffer.Update(data); }
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const { return buffer.GetGPUVirtualAddress(); }
};