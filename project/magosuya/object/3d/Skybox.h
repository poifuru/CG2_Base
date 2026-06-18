//#pragma once
//#include <Windows.h>
//#include <Wrl.h>
//using namespace Microsoft::WRL;
//#include <d3d12.h>
//#include <string>
//#include "struct.h"
//#include "PSOManager.h"
//#include "DxCommon.h"
//#include "CameraComponent.h"
//
//struct SkyboxVertex {
//	Vector4 position;
//};
//
//class Skybox {
//public:
//	Skybox(DxCommon* dxCommon);
//
//	void Initialize(std::string filePath, std::string tag);
//
//	void Update(CameraData* data);
//
//	void Draw();
//
//private:
//	// PSO
//	PSODescriptor desc_{};
//
//	// バッファ
//	ComPtr<ID3D12Resource> vertexBuffer_ = nullptr;
//	D3D12_VERTEX_BUFFER_VIEW vbv_{};
//	ComPtr<ID3D12Resource> indexBuffer_ = nullptr;
//	D3D12_INDEX_BUFFER_VIEW ibv_{};
//	ComPtr<ID3D12Resource> matrixBuffer_ = nullptr;
//	ComPtr<ID3D12Resource> materialBuffer_ = nullptr;
//
//	// マッピング用のポインタ
//	SkyboxVertex* vertexData_ = nullptr;
//	uint32_t* indexData_ = nullptr;
//	TransformationMatrix* matrixData_ = nullptr;
//	Material* materialData_ = nullptr;
//
//	// テクスチャを引っ張てくるための変数
//	std::string tag_;
//
//	// 借りるポインタ
//	DxCommon* dxCommon_ = nullptr;
//	ID3D12Device* device_ = nullptr;
//	ID3D12GraphicsCommandList* commandList_ = nullptr;
//};