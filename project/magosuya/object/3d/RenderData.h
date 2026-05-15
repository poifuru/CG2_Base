#pragma once
#include "struct.h"
#include "DxCommon.h"

// 汎用的な頂点データの構造体
struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

#pragma region モデル描画に必要な最小限の頂点データ
struct ModelData {		// CPU
	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;
};

struct ModelResource {	// GPU
	// 頂点バッファ
	ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vbv;

	// インデックスバッファ
	ComPtr<ID3D12Resource> indexBuffer;
	D3D12_INDEX_BUFFER_VIEW ibv;
};
#pragma endregion

#pragma region マテリアルデータ
struct MaterialData {		// CPU
	Vector4 color;
	Matrix4x4 uvTransform;
	float roughness; // 粗さ
	float metallic; // 金属度
	float environmentCoefficient;	// 環境係数
	BOOL enableLighting;
};
#pragma endregion

// 頂点バッファ専用のテンプレートクラス
template <typename T>
class VertexBuffer {
public:
	void Initialize(DxCommon* dxCommon, const std::vector<T>& vertices) {
		size_ = sizeof(T) * vertices.size();
		buffer_ = dxCommon->CreateBufferResource(size_);

		// データを書き込みとマッピング
		void mappedData = nullptr;
		if(SUCCEEDED(buffer_->Map(0, nullptr, &mappedData))) {
			std::memcpy(mappedData, vertices.data(), size_);
			buffer_->Unmap(0, nullptr);
		}

		// ビューの設定も済ませる
		vbv_.BufferLocation = buffer_->GetGPUVirtualAddress();
		vbv_.SizeInBytes = static_cast<UINT>(size_);
		vbv_.StrideInBytes = sizeof(T);
	}

	void Update(const T& data) {

	}

	const D3D12_VERTEX_BUFFER_VIEW& GetView() const { return vbv_; }

private:
	ComPtr<ID3D12Resource> buffer_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vbv_{};
	size_t size_ = 0;
};

// 定数バッファのテンプレートクラス
template <typename T>
class ConstantBuffer {
public:
	ConstantBuffer() = default;	// デフォルトコンストラクタ

	// デストラクタ時に自動でUnmap
	~ConstantBuffer() {
		Release();
	}

	void Initialize(DxCommon* dxCommon) {
		Release();

		// sizeof(MaterialData) だった部分を sizeof(T) にすることで、どんな型にも対応！
		buffer_ = dxCommon->CreateBufferResource(sizeof(T));
		if(buffer_) {
			HRESULT hr = buffer_->Map(0, nullptr, &mappedData_);
			if(FAILED(hr)) {
				mappedData_ = nullptr;
			}
		}
	}

	void Release() {
		if(buffer_ && mappedData_) {
			buffer_->Unmap(0, nullptr);
		}
		buffer_ = nullptr;
		mappedData_ = nullptr;
	}

	void Update(const T& data) {
		if(mappedData_) {
			std::memcpy(mappedData_, &data, sizeof(T));
		}
	}

	// GPUのアドレス位置を取得する関数
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const {
		if(buffer_) {
			return buffer_->GetGPUVirtualAddress();
		}
		return 0;
	}

public:
	// コピー禁止（ポインタの二重管理を防ぐためにめちゃくちゃ重要！）
	ConstantBuffer(const ConstantBuffer&) = delete;
	ConstantBuffer& operator=(const ConstantBuffer&) = delete;

	// ムーブは許可（必要なら）
	ConstantBuffer(ConstantBuffer&& other) noexcept {
		*this = std::move(other);
	}
	ConstantBuffer& operator=(ConstantBuffer&& other) noexcept {
		if(this != &other) {
			Release();
			buffer_ = std::move(other.buffer_);
			mappedData_ = other.mappedData_;
			other.mappedData_ = nullptr;
		}
		return *this;
	}

private:
	ComPtr<ID3D12Resource> buffer_;
	void* mappedData_ = nullptr;		// マップされたポインタを常に持っておく
};