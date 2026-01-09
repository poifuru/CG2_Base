#pragma once
#include <Windows.h>
#include <Wrl.h>
using namespace Microsoft::WRL;
#include <d3d12.h>
#include <vector>
#include <string>
#include "struct.h"
#include "PSOManager.h"
#include "DxCommon.h"

class ModelRenderer {
public:
	ModelRenderer (DxCommon* dxCommon);
	~ModelRenderer ();

	void Initialize ();
	void Update (Matrix4x4 world, Matrix4x4 vp, Transform uvTransform);
	void Draw (D3D12_GPU_DESCRIPTOR_HANDLE textureHandle, ID3D12Resource* light);
	void ImGui (Transform& transform, Transform& uvTransform, const std::string& windowName);

	//アクセッサ
	Material* GetMaterial () { return materialData_; }
	void IsLighting (const LightReflectionModel& lighting) { materialData_->enableLighting = lighting; }
	void SetColor (const Vector4& color) { materialData_->color = color; }
	void SetAlpha(const float& alpha) { materialData_->color.w = alpha; }
	void SetImGuiID (const std::string& id) { tag_ = id; }
	void SetModelData (const std::weak_ptr<ModelData>& data){ modelData_ = data; }

private:
	//モデルデータ
	std::weak_ptr<ModelData> modelData_;

	//PSO
	PSODescriptor desc_ = {};

	//GPUリソース
	ComPtr<ID3D12Resource> matrixBuffer_;
	ComPtr<ID3D12Resource> materialBuffer_;

	//GPUリソースにマッピングするデータ
	TransformationMatrix* matrixData_ = nullptr;
	Material* materialData_ = nullptr;

	//ImGui用のラベル名
	std::string tag_;
	int instanceID_ = 0;
	static inline int modelCount_ = 0;

	//ImGuiで色をいじる変数
	float color_[4];

	//ポインタを借りる
	DxCommon* dxCommon_ = nullptr;
	ID3D12GraphicsCommandList* commandList_ = nullptr;
};