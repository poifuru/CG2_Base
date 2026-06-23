#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <cassert>

// 定数バッファのテンプレートクラス
template <typename T>
class ConstantBuffer {
public:
	ConstantBuffer() = default;
	~ConstantBuffer() { Release(); }

	// 初期化
	void Initialize(ID3D12Device* device) {
		Release();
		assert(device != nullptr);

		// 定数バッファは256バイトの倍数でアロケーションする
		sizeInBytes_ = (sizeof(T) + 255) & ~255;

		D3D12_HEAP_PROPERTIES uploadHeapProperties = {};
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Width = sizeInBytes_;
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
	}

	// 更新
	void Update(const T& data) {
		if (mappedData_) {
			std::memcpy(mappedData_, &data, sizeof(T));
		}
	}

	// 解放
	void Release() {
		if (buffer_ && mappedData_) {
			buffer_->Unmap(0, nullptr);
		}
		buffer_ = nullptr;
		mappedData_ = nullptr;
	}

	// アクセッサ
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const { return buffer_ ? buffer_->GetGPUVirtualAddress() : 0; }

public:
	// コピー禁止（ポインタの二重管理を防ぐため）
	ConstantBuffer(const ConstantBuffer&) = delete;
	ConstantBuffer& operator=(const ConstantBuffer&) = delete;

	// ムーブは許可
	ConstantBuffer(ConstantBuffer&& other) noexcept { *this = std::move(other); }
	ConstantBuffer& operator=(ConstantBuffer&& other) noexcept {
		if (this != &other) {
			Release();
			buffer_ = std::move(other.buffer_);
			mappedData_ = other.mappedData_;
			sizeInBytes_ = other.sizeInBytes_;
			other.mappedData_ = nullptr;
		}
		return *this;
	}

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> buffer_ = nullptr;
	void* mappedData_ = nullptr;
	size_t sizeInBytes_ = 0;
};
