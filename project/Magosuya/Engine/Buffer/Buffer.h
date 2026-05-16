#pragma once
#include <Windows.h>
#include <wrl.h>
using namespace Microsoft::WRL;
#include <d3d12.h>
#include <vector>
#include <string>
#include <map>
#include <optional>
#include <span>
#include <array>

#include "struct.h"
#include "DxCommon.h"
#include "DirectXTex.h"

// ModelData構造体
//struct ModelData {
//	// 形状情報 (CPU側データ)
//	std::string materialFilePath;
//	std::vector<VertexData> vertices;
//	size_t vertexCount = 0;
//
//	// インデックス描画用のCPU側データ
//	std::vector<uint32_t> indices;
//	size_t indexCount = 0;
//
//	// Dxリソース (GPU側データ) インスタンス間で共有される
//	// 頂点バッファ
//	ComPtr<ID3D12Resource> vertexBuffer;
//	D3D12_VERTEX_BUFFER_VIEW vbView{};
//
//	// インデックスバッファ
//	ComPtr<ID3D12Resource> indexBuffer;
//	D3D12_INDEX_BUFFER_VIEW ibView{};
//
//	// ルートノード(階層構造)
//	Node rootNode;
//
//	// スキンクラスターのデータ
//	std::map<std::string, JointWeightData> skinClusterData;
//};

// 頂点バッファ専用のテンプレートクラス
template <typename T>
class VertexBuffer {
public:
	/// <summary>
	/// デフォルトコンストラクタ
	/// </summary>
	VertexBuffer() = default;

	/// <summary>
	/// デストラクタ
	/// </summary>
	~VertexBuffer() { Release(); }

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="dxCommon">dxCommonのインスタンス</param>
	/// <param name="vertices">描画するモデルの頂点データ配列</param>
	void Initialize(DxCommon* dxCommon, const std::vector<T>& vertices) {
		Release();

		size_ = sizeof(T) * vertices.size();
		buffer_ = dxCommon->CreateBufferResource(size_);

		// データを書き込みとマッピング
		if(buffer_) {
			HRESULT hr = buffer_->Map(0, nullptr, &mappedData_);
			if(FAILED(hr)) {
				mappedData_ = nullptr;
			}
		}

		// ビューの設定も済ませる
		vbv_.BufferLocation = buffer_->GetGPUVirtualAddress();
		vbv_.SizeInBytes = static_cast<UINT>(size_);
		vbv_.StrideInBytes = sizeof(T);
	}

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="vertices">更新する頂点データ配列</param>
	void Update(const std::vector<T>& vertices) {
		if(mappedData_ && !vertices.empty()) {
			// 確保したバッファサイズを超えないようにコピー
			size_t copySize = std::min(size_, sizeof(T) * vertices.size());
			std::memcpy(mappedData_, vertices.data(), copySize);
		}
	}

	/// <summary>
	/// リソースの解放
	/// </summary>
	void Release() {
		if(buffer_ && mappedData_) {
			buffer_->Unmap(0, nullptr);
		}
		buffer_ = nullptr;
		mappedData_ = nullptr;
	}

	/// <summary>
	/// 頂点バッファビューを返す
	/// </summary>
	/// <returns>VBV</returns>
	const D3D12_VERTEX_BUFFER_VIEW& GetView() const { return vbv_; }

public:
	// コピー禁止（ポインタの二重管理を防ぐため）
	VertexBuffer(const VertexBuffer&) = delete;
	VertexBuffer& operator=(const VertexBuffer&) = delete;

	// ムーブは許可
	VertexBuffer(VertexBuffer&& other) noexcept {
		*this = std::move(other);
	}
	VertexBuffer& operator=(VertexBuffer&& other) noexcept {
		if(this != &other) {
			Release();
			buffer_ = std::move(other.buffer_);
			mappedData_ = other.mappedData_;
			other.mappedData_ = nullptr;
		}
		return *this;
	}

private:
	ComPtr<ID3D12Resource> buffer_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vbv_{};
	size_t size_ = 0;
	void* mappedData_ = nullptr;	// マップ用のポインタを常に持っておく
};

template <typename T>
class IndexBuffer {
public:
	/// <summary>
	/// デフォルトコンストラクタ
	/// </summary>
	IndexBuffer() = default;

	/// <summary>
	/// デストラクタ
	/// </summary>
	~IndexBuffer() {
		Release();
	}

	void Initialize(DxCommon* dxCommon, size_t maxIndices) {
		Release();

		size_ = sizeof(T) * maxIndices;
		buffer_ = dxCommon->CreateBufferResource(size_);

		if(buffer_) {
			// マッピング開始
			HRESULT hr = buffer_->Map(0, nullptr, &mappedData_);
			if(FAILED(hr)) {
				mappedData_ = nullptr;
			}
		}

		// ビューの設定
		ibv_.BufferLocation = buffer_->GetGPUVirtualAddress();
		ibv_.SizeInBytes = static_cast<UINT>(size_);
		// テンプレート引数を見て自動でフォーマット切り替え
		// uint32_t(4バイト)ならR32_UINT、uint16_t(2バイト)ならR16_UINT
		ibv_.Format = (sizeof(4)) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UITN;
	}

	void Update(const std::vector<T>& indices) {
		if(mappedData_ && !indices.enpty()) {
			// 確保したバッファサイズを超えないようにコピー
			size_t copySize = std::min(size_, sizeof(T) * indices.size());
			std::memcpy(mappedData_, indices.data(), copySize);
		}
	}

	void Release() {
		if(buffer_ && mappedData_) {
			buffer_->Unmap(0, nullptr);
		}
		buffer_ = nullptr;
		mappedData_ = nullptr;
	}

	const D3D12_INDEX_BUFFER_VIEW GetView() { return ibv_; }

public:
	// コピー禁止
	IndexBuffer(const IndexBuffer&) = delete;
	IndexBuffer& operator=(const IndexBuffer&) = delete;

	// ムーブ許可
	IndexBuffer(IndexBuffer&& other) noexcept {
		*this = std::move(other);
	}
	IndexBuffer& operator=(IndexBuffer&& other) noexcept {
		if(this != &other) {
			Release();
			buffer_ = std::move(other.buffer_);
			mappedData_ = other.mappedData_;
			other.mappedData_ = nullptr;
			ibv_ = other.ibv_; // ビューの情報も忘れずにコピー
		}
		return *this;
	}

private:
	ComPtr<ID3D12Resource> buffer_ = nullptr;
	D3D12_INDEX_BUFFER_VIEW ibv_{};
	size_t size_ = 0;
	void* mappedData_ = nullptr; // マップ用のポインタを常に持っておく
};

// 定数バッファのテンプレートクラス
template <typename T>
class ConstantBuffer {
public:
	/// <summary>
	/// デフォルトコンストラクタ
	/// </summary>
	ConstantBuffer() = default;

	/// <summary>
	/// デストラクタ
	/// </summary>
	~ConstantBuffer() {
		Release();
	}

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="dxCommon">dxCommonのインスタンス</param>
	void Initialize(DxCommon* dxCommon) {
		Release();

		// バッファを作成してマッピング
		buffer_ = dxCommon->CreateBufferResource(sizeof(T));
		if(buffer_) {
			HRESULT hr = buffer_->Map(0, nullptr, &mappedData_);
			if(FAILED(hr)) {
				mappedData_ = nullptr;
			}
		}
	}

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="data">バッファにコピーするデータ</param>
	void Update(const T& data) {
		if(mappedData_) {
			// 引数で受け取った値をmappedData_にコピー
			std::memcpy(mappedData_, &data, sizeof(T));
		}
	}

	/// <summary>
	/// リソースを解放する
	/// </summary>
	void Release() {
		if(buffer_ && mappedData_) {
			buffer_->Unmap(0, nullptr);
		}
		buffer_ = nullptr;
		mappedData_ = nullptr;
	}

	/// <summary>
	/// GPUのアドレス位置を取得する
	/// </summary>
	/// <returns>GPUアドレス</returns>
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const {
		if(buffer_) {
			return buffer_->GetGPUVirtualAddress();
		}
		return 0;
	}

public:
	// コピー禁止（ポインタの二重管理を防ぐため）
	ConstantBuffer(const ConstantBuffer&) = delete;
	ConstantBuffer& operator=(const ConstantBuffer&) = delete;

	// ムーブは許可
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
	void* mappedData_ = nullptr;	// マップ用のポインタを常に持っておく
};

// 構造化バッファのクラステンプレート
template <typename T>
class StructuredBuffer {
public:
	StructuredBuffer() = default;

	~StructuredBuffer() { Release(); }

public:


private:
	ComPtr<ID3D12Resource> bufferResource_;
	void* mappedData_ = nullptr;
	size_t elementCount = 0;
};