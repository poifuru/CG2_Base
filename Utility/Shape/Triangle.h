#pragma once
#include "../../externals/DirectXTex/d3dx12.h"
#include "../../header/struct.h"
#include "../../header/ComPtr.h"

class Triangle {
public:
	/// <summary>
	/// 三角形描画に必要なデータを作る
	/// </summary>
	void Initialize (ID3D12Device* device);

	/// <summary>
	/// 実際にデータを詰める
	/// </summary>
	void Update (ShapeData* shapeData);

	/// <summary>
	/// 描画コマンドを詰む
	/// </summary>
	void Command (ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE* textureSrvHandleGPU);

private:
	//GPUリソース  
	ComPtr<ID3D12Resource> vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vbView_{};
	ComPtr<ID3D12Resource> matrixBuffer_;
	ComPtr<ID3D12Resource> materialBuffer_;
	ComPtr<ID3D12Resource> lightBuffer_;

	ShapeData* shapeData_ = nullptr;
};