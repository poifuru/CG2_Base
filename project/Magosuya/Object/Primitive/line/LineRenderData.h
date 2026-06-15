#pragma once
#include "struct.h"
#include "Buffer.h"

struct LineData {
	Vector3 position;
	float padding;
	Vector4 color;
};

struct LineVertexData {
	//頂点バッファ
	ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vbView{};
};

struct LineForGPU {
	Matrix4x4 World;
	Matrix4x4 WVP;
};