#pragma once
#include "Buffer.h"

// 行列データ
struct MatrixData {
	Matrix4x4 World;
	Matrix4x4 WVP;
	Matrix4x4 WorldInverseTranspose;	// ライティングの時に正しい法線を計算する
};

struct MatrixResource {
	ConstantBuffer<MatrixData> buffer;

	// メソッド
	void Initialize(DxCommon* dxCommon) { buffer.Initialize(dxCommon); }
	void Update(const MatrixData& data) { buffer.Update(data); }
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const { return buffer.GetGPUVirtualAddress(); }
};