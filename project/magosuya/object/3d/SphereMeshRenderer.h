//#pragma once
//#include <Windows.h>
//#include <Wrl.h>
//using namespace Microsoft::WRL;
//#include <d3d12.h>
//#include <vector>
//#include <string>
//#include "struct.h"
//#include "PSOManager.h"
//#include "DxCommon.h"
//
//class SphereMeshRenderer {
//public:
//	SphereMeshRenderer(DxCommon* dxCommon);
//	~SphereMeshRenderer();
//
//	void Initialize(float radius);
//	void Update(Matrix4x4 world, Matrix4x4 vp, Transform uvTransform);
//	void Draw(D3D12_GPU_DESCRIPTOR_HANDLE textureHandle, ID3D12Resource* light);
//	void ImGui(Transform& transform, Transform& uvTransform, const std::string& windowName);
//
//	//アクセッサ
//	Material* GetMaterial() { return materialData_; }
//	void IsLighting(const LightReflectionModel& lighting) { materialData_->enableLighting = lighting; }
//	void SetColor(const Vector4& color) { materialData_->color = color; }
//	void SetAlpha(const float& alpha) { materialData_->color.w = alpha; }
//	void SetImGuiID(const std::string& id) { tag_ = id; }
//
//private:
//	//PSO
//	PSODescriptor desc_ = {};
//
//	//GPUリソース
//	ComPtr<ID3D12Resource> vertexBuffer_;
//	//ComPtr<ID3D12Resource> indexBuffer_;
//	ComPtr<ID3D12Resource> matrixBuffer_;
//	ComPtr<ID3D12Resource> materialBuffer_;
//
//	//GPUリソースにマッピングするデータ
//	std::vector<VertexData> vertexData_;
//	VertexData* vertexDataPtr_;    // GPU側への書き込みポインタ
//	//uint32_t* indexData_;
//	TransformationMatrix* matrixData_ = nullptr;
//	Material* materialData_ = nullptr;
//
//	//バッファビュー
//	D3D12_VERTEX_BUFFER_VIEW vbView_{};
//	D3D12_INDEX_BUFFER_VIEW ibView_{};
//
//	//球の半径
//	float radius_;
//
//	//球の縦横の分割数
//	int kSubdivision_;
//
//	//ImGui用のラベル名
//	std::string tag_;
//	int instanceID_ = 0;
//	static inline int modelCount_ = 0;
//
//	//ImGuiで色をいじる変数
//	float color_[4];
//
//	//ポインタを借りる
//	DxCommon* dxCommon_ = nullptr;
//	ID3D12GraphicsCommandList* commandList_ = nullptr;
//};