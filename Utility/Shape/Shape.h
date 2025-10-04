#pragma once
#include "../../header/struct.h"
#include "../../externals/DirectXTex/d3dx12.h"
#include "../../header/ComPtr.h"
#include "Triangle.h"

class Shape {
public:
	void Initialize (ID3D12Device* device, ID3D12GraphicsCommandList* commandList);

	//とりあえず出せるように
	void DrawTriangle (ShapeData* shapeData, D3D12_GPU_DESCRIPTOR_HANDLE* textureSrvHandleGPU);

private:
	ComPtr<ID3D12Device> device_;
	ComPtr<ID3D12GraphicsCommandList> commandList_;

	Triangle triangle_;
};

