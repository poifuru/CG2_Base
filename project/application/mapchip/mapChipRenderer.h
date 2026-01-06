#pragma once
#include <Windows.h>
#include <Wrl.h>
using namespace Microsoft::WRL;
#include <d3d12.h>
#include <memory>
#include <map>
#include "mapchip.h"
#include "struct.h"
#include "Model.h"
#include "PSOManager.h"

struct MapforGPU {
	Matrix4x4 wvp;
	Matrix4x4 world;
	Vector4 color;
};

struct InstancingResource {
	ComPtr<ID3D12Resource> resource;
	MapforGPU* mappedData;
	std::vector<MapforGPU> cpuData;

	int srvIndex;
};

class MapChipRenderer {
public:
	void Initialize();
	void Update(MapChip& mapChip, const Matrix4x4& vp);
	void Draw();

private:
	//マップの種類ごとのリソース
	std::map<MapChipType, InstancingResource> resources_;

	//頂点バッファ
	ComPtr<ID3D12Resource> vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vbView_{};

	//インデックスバッファ
	ComPtr<ID3D12Resource> indexBuffer_;
	D3D12_INDEX_BUFFER_VIEW ibView_{};

	//マテリアルバッファ
	ComPtr<ID3D12Resource> materialBuffer_;

	//マッピング用のCPUデータ
	VertexData* vertexData_ = nullptr;
	uint32_t* indexData_ = nullptr;
	Material* materialData_ = nullptr;

	//PSOの設定
	PSODescriptor desc_;
};