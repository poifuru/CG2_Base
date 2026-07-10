#pragma once
#define NOMINMAX // std::min, std::maxを<Windows.h>に勝手に定義させない
#include <Windows.h>
#include <d3d12.h>
#include <vector>
#include <algorithm>
#include "DxCommon.h"
#include "SRVManager.h"
#include "DescriptorHandle.h"

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
			size_t copySize = (std::min)(size_, sizeof(T) * vertices.size());
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

	/// <summary>
	/// リソースの生ポインタを返す
	/// </summary>
	/// <returns>ID3D12Resource*</returns>
	ID3D12Resource* GetResource() const { return buffer_.Get(); }

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
		ibv_.Format = (sizeof(T) == 4) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
	}

	void Update(const std::vector<T>& indices) {
		if(mappedData_ && !indices.empty()) {
			// 確保したバッファサイズを超えないようにコピー
			size_t copySize = (std::min)(size_, sizeof(T) * indices.size());
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

	// 要素数(count)を指定して初期化
	void Initialize(DxCommon* dxCommon, size_t count) {
		Release();
		elementCount_ = count;
		size_t bufferSize = sizeof(T) * elementCount_;

		// GPUリソース生成
		buffer_ = dxCommon->CreateBufferResource(bufferSize);
		if(buffer_) {
			buffer_->Map(0, nullptr, &mappedData_);
		}

		// SRVの生成処理
		// 空いているディスクリプタのインデックスを確保
		srvIndex_ = SRVManager::GetInstance()->Allocate();

		// SRVManagerに構造化バッファ用のSRVを作ってもらう
		SRVManager::GetInstance()->CreateSRVStructuredBuffer(srvIndex_, buffer_.Get(), static_cast<UINT>(elementCount_), static_cast<UINT>(sizeof(T)));

		// 描画時にコマンドリストへ渡すためのGPUハンドルを取得して保持しておく
		srvHandle_.gpu = SRVManager::GetInstance()->GetGPUDescriptorHandle(srvIndex_);
	}

	// UAV(かつSRV)として初期化
	void InitializeUAV(DxCommon* dxCommon, size_t count) {
		Release();
		elementCount_ = count;
		size_t bufferSize = sizeof(T) * elementCount_;

		ID3D12Device* device = dxCommon->GetDevice();

		// Default Heapにリソース作成（UAVを許可）
		D3D12_HEAP_PROPERTIES heapProperties = {};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 1;
		heapProperties.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = 0;
		desc.Width = bufferSize;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		HRESULT hr = device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&buffer_)
		);
		assert(SUCCEEDED(hr));

		// SRVの生成
		srvIndex_ = SRVManager::GetInstance()->Allocate();
		SRVManager::GetInstance()->CreateSRVStructuredBuffer(srvIndex_, buffer_.Get(), static_cast<UINT>(elementCount_), static_cast<UINT>(sizeof(T)));
		srvHandle_.gpu = SRVManager::GetInstance()->GetGPUDescriptorHandle(srvIndex_);

		// UAVの生成
		uavIndex_ = SRVManager::GetInstance()->Allocate();
		SRVManager::GetInstance()->CreateUAVStructuredBuffer(uavIndex_, buffer_.Get(), static_cast<UINT>(elementCount_), static_cast<UINT>(sizeof(T)));
		uavHandle_.gpu = SRVManager::GetInstance()->GetGPUDescriptorHandle(uavIndex_);
		hasUAV_ = true;
	}
		
	void Update(const std::vector<T>& data) {
		if(!mappedData_ || data.empty()) return;
		// 安全のためにサイズチェック
		size_t copyCount = (std::min)(elementCount_, data.size());
		std::memcpy(mappedData_, data.data(), sizeof(T) * copyCount);
	}

	void Release() {
		// buffer_ が存在している場合のみ解放する
		if(buffer_) {
			if(mappedData_) {
				buffer_->Unmap(0, nullptr);
			}
			SRVManager::GetInstance()->Free(srvIndex_);
			if (hasUAV_) {
				SRVManager::GetInstance()->Free(uavIndex_);
			}
		}
		buffer_ = nullptr;
		mappedData_ = nullptr;
		elementCount_ = 0;
		// 念のため初期値に戻す
		srvIndex_ = 0;
		uavIndex_ = 0;
		hasUAV_ = false;
	}

	// SRV作成時に必要な情報をゲッターで提供
	ID3D12Resource* GetResource() const { return buffer_.Get(); }
	size_t GetElementCount() const { return elementCount_; }
	UINT GetStride() const { return static_cast<UINT>(sizeof(T)); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVHandle() const { return srvHandle_.gpu; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetUAVHandle() const { return uavHandle_.gpu; }

public:
	// コピー禁止（ポインタの二重管理を防ぐため）
	StructuredBuffer(const StructuredBuffer&) = delete;
	StructuredBuffer& operator=(const StructuredBuffer&) = delete;

	// ムーブは許可
	StructuredBuffer(StructuredBuffer&& other) noexcept {
		*this = std::move(other);
	}
	StructuredBuffer& operator=(StructuredBuffer&& other) noexcept {
		if(this != &other) {
			Release();
			buffer_ = std::move(other.buffer_);
			mappedData_ = other.mappedData_;
			elementCount_ = other.elementCount_;
			srvIndex_ = other.srvIndex_;
			srvHandle_ = other.srvHandle_;
			uavIndex_ = other.uavIndex_;
			uavHandle_ = other.uavHandle_;
			hasUAV_ = other.hasUAV_;

			// ムーブ元を空にする
			other.buffer_ = nullptr;
			other.mappedData_ = nullptr;
			other.elementCount_ = 0;
			other.srvIndex_ = 0;
			other.uavIndex_ = 0;
			other.hasUAV_ = false;
		}
		return *this;
	}

private:
	ComPtr<ID3D12Resource> buffer_;
	void* mappedData_ = nullptr;
	size_t elementCount_ = 0;

	// SRV管理用の変数を追加
	UINT srvIndex_ = 0;
	DescriptorHandle srvHandle_{};

	// UAV管理用の変数を追加
	UINT uavIndex_ = 0;
	DescriptorHandle uavHandle_{};
	bool hasUAV_ = false;
};