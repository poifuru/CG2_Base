#pragma once

template <typename T>
class IndexBuffer {
public:
	IndexBuffer() = default;
	~IndexBuffer() { Release(); }

	// 初期化
	void Initialize(ID3D12Device* device, size_t maxIndices) {
		Release();
		assert(device != nullptr);

		maxIndices_ = maxIndices;
		size_ = sizeof(T) * maxIndices_;

		D3D12_HEAP_PROPERTIES uploadHeapProperties = {};
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

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

		hr = buffer_->Map(0, nullptr, &mappedData_);
		assert(SUCCEEDED(hr));

		// ビューの設定
		ibv_.BufferLocation = buffer_->GetGPUVirtualAddress();
		ibv_.SizeInBytes = static_cast<UINT>(size_);
		// uint32_tならR32_UINT、uint16_tならR16_UINT
		ibv_.Format = (sizeof(T) == 4) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
	}

	//　更新
	void Update(const std::vector<T>& indices) {
		if (mappedData_ && !indices.empty()) {
			size_t copySize = (std::min)(size_, sizeof(T) * indices.size());
			std::memcpy(mappedData_, indices.data(), copySize);
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
		maxIndices_ = 0;
	}

	// アクセッサ
	const D3D12_INDEX_BUFFER_VIEW& GetView() const { return ibv_; }
	void SetView(const D3D12_INDEX_BUFFER_VIEW& ibv) { return ibv_ = ibv; }

	UINT GetIndexCount() const { return static_cast<UINT>(maxIndices_); }

public:
	// コピー禁止
	IndexBuffer(const IndexBuffer&) = delete;
	IndexBuffer& operator=(const IndexBuffer&) = delete;

	// ムーブ許可
	IndexBuffer(IndexBuffer&& other) noexcept { *this = std::move(other); }
	IndexBuffer& operator=(IndexBuffer&& other) noexcept {
		if (this != &other) {
			Release();
			buffer_ = std::move(other.buffer_);
			mappedData_ = other.mappedData_;
			ibv_ = other.ibv_;
			size_ = other.size_;
			maxIndices_ = other.maxIndices_;
			other.mappedData_ = nullptr;
		}
		return *this;
	}

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> buffer_ = nullptr;
	D3D12_INDEX_BUFFER_VIEW ibv_{};
	size_t size_ = 0;
	size_t maxIndices_ = 0;
	void* mappedData_ = nullptr;
};