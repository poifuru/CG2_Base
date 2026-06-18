//#pragma once
//#include <Windows.h>
//#include <Wrl.h>
//using namespace Microsoft::WRL;
//#include <d3d12.h>
//#include <memory>
//#include <map>
//#include "MapchipStruct.h"
//#include "Model.h"
//#include "DxCommon.h"
//#include "PSOManager.h"
//#include "lightManager.h"
//
//class MapChip;
//
//class MapChipRenderer {
//public:
//	void Initialize(DxCommon* dxCommon, LightManager* lightManager);
//	void Update(MapChip& mapChip, const Matrix4x4& vp, Vector3 cameraWorld);
//	void Draw();
//	void ImGui(const std::string& windowName);
//
//private:
//	//マップの種類ごとのリソース
//	std::map<MapChipType, InstancingResource> resources_;
//
//	//頂点バッファ
//	ComPtr<ID3D12Resource> vertexBuffer_;
//	D3D12_VERTEX_BUFFER_VIEW vbView_{};
//
//	//インデックスバッファ
//	ComPtr<ID3D12Resource> indexBuffer_;
//	D3D12_INDEX_BUFFER_VIEW ibView_{};
//
//	//マテリアルバッファ
//	ComPtr<ID3D12Resource> materialBuffer_;
//	ComPtr<ID3D12Resource> cameraBuffer_;
//
//	//マッピング用のCPUデータ
//	VertexData* vertexData_ = nullptr;
//	uint32_t* indexData_ = nullptr;
//	Material* materialData_ = nullptr;
//	Vector3* cameraData_ = nullptr;
//
//	//PSOの設定
//	PSODescriptor desc_;
//
//	//ポインタ
//	DxCommon* dxCommon_ = nullptr;
//	ID3D12GraphicsCommandList* commandList_ = nullptr;
//	LightManager* lightManager_ = nullptr;
//};