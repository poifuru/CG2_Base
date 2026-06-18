#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <algorithm>
#include <cassert>

// 頂点バッファ専用のテンプレートクラス
template <typename T>
class VertexBuffer {
public:
	VertexBuffer() = default;
	~VertexBuffer() { Release(); }

	// 初期化
	void Initialize(ID3D12Device* device, size_t maxVertices) {
		Release();
		assert(device != nullptr);

		maxVertices_ = maxVertices;
		size_ = sizeof(T) * maxVertices_;

		// アップロードヒープ（CPUから書き込めるVRAM領域）の設定
		D3D12_HEAP_PROPERTIES uploadHeapProperties = {};
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

		// 純粋なバッファとしてリソースを定義
		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Width = size_;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		HRESULT hr = device->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(buffer_.GetAddressOf())
		);
		assert(SUCCEEDED(hr));

		// マップしてポインタを握っておく
		hr = buffer_->Map(0, nullptr, &mappedData_);
		assert(SUCCEEDED(hr));

		// 頂点バッファビュー(VBV)の構築
		vbv_.BufferLocation = buffer_->GetGPUVirtualAddress();
		vbv_.SizeInBytes = static_cast<UINT>(size_);
		vbv_.StrideInBytes = sizeof(T);
	}

	// 更新
	void Update(const std::vector<T>& vertices) {
		if (mappedData_ && !vertices.empty()) {
			size_t copySize = (std::min)(size_, sizeof(T) * vertices.size());
			std::memcpy(mappedData_, vertices.data(), copySize);
		}
	}

	// 解放
	void Release() {
		if (buffer_ && mappedData_) {
			buffer_->Unmap(0, nullptr);
		}
		buffer_ = nullptr;
		mappedData_ = nullptr;
		size_ = 0;
		maxVertices_ = 0;
	}

	// アクセッサ
	const D3D12_VERTEX_BUFFER_VIEW& GetView() const { return vbv_; }

public:
	// コピー禁止（ポインタの二重管理を防ぐため）
	VertexBuffer(const VertexBuffer&) = delete;
	VertexBuffer& operator=(const VertexBuffer&) = delete;

	VertexBuffer(VertexBuffer&& other) noexcept { *this = std::move(other); }
	VertexBuffer& operator=(VertexBuffer&& other) noexcept {
		if (this != &other) {
			Release();
			buffer_ = std::move(other.buffer_);
			mappedData_ = other.mappedData_;
			vbv_ = other.vbv_;
			size_ = other.size_;
			maxVertices_ = other.maxVertices_;
			other.mappedData_ = nullptr;
		}
		return *this;
	}

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> buffer_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vbv_{};
	size_t size_ = 0;
	size_t maxVertices_ = 0;
	void* mappedData_ = nullptr;
};