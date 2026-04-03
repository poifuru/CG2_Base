#pragma once
#include <Windows.h>
#include <wrl.h>
using namespace Microsoft::WRL;
#include <d3d12.h>
#include "struct.h"
#include "PSOManager.h"

class DxCommon;
class TextureManager;

class SpriteRenderer {
public:		//外部公開メソッド
	SpriteRenderer (DxCommon* dxCommon);
	~SpriteRenderer ();

	void Initialize ();
	void Update (Matrix4x4 wvpData, EulerTransform uvTransform, Vector2 anchorPoint,
				 bool flipX, bool flipY, const std::string& id, Vector2 texLeftTop, Vector2 texSize);
	void Draw (D3D12_GPU_DESCRIPTOR_HANDLE textureHandle);
	void ImGui (EulerTransform& transform, EulerTransform& uvTransform);

	//アクセッサ
	Material* GetMaterial () { return materialData_; }
	void SetColor (const Vector4& color) { materialData_->color = color; }
	void SetID (const std::string& id) { id_ = id; }

private:	//メンバ変数
	PSODescriptor desc_ = {};

	//GPUリソース
	ComPtr<ID3D12Resource> vertexBuffer_;
	ComPtr<ID3D12Resource> indexBuffer_;
	ComPtr<ID3D12Resource> matrixBuffer_;
	ComPtr<ID3D12Resource> materialBuffer_;

	D3D12_VERTEX_BUFFER_VIEW vbView_{};
	D3D12_INDEX_BUFFER_VIEW ibView_{};

	//GPUリソースにマッピングするデータ
	VertexData* vertexData_ = nullptr;
	uint32_t* indexData_ = nullptr;
	Matrix4x4* matrixData_ = nullptr;
	Material* materialData_ = nullptr;

	//画像検索用のID
	std::string id_;

	//ImGuiで色をいじる
	float color_[4];

	//ポインタを借りる
	DxCommon* dxCommon_ = nullptr;
	ID3D12GraphicsCommandList* commandList_ = nullptr;
	TextureManager* texManager_ = nullptr;
};

