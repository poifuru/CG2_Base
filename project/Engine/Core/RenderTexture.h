#pragma once

namespace MyEngine::LowLevel {
	class DescriptorHeapManager;
}

namespace MyEngine::Rendering {
	class RenderTexture {
	public:
		RenderTexture() = default;
		~RenderTexture() = default;

		/// <summary>
		/// 初期化
		/// </summary>
		void Initialize(ID3D12Device* device, MyEngine::LowLevel::DescriptorHeapManager* heapManager);

		/// <summary>
		/// RenderTexture生成関数
		/// </summary>
		Microsoft::WRL::ComPtr<ID3D12Resource> CreateRenderTextureResource(
			ID3D12Device* device, uint32_t width, uint32_t height, DXGI_FORMAT format, const Vector4& clearColor
		);

		// アクセッサ
		ID3D12Resource* GetResource() const { return resource_.Get(); }
		D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle() const { return rtvHandle_; }
		uint32_t GetSrvIndex() const { return srvIndex_; }

	private:
		// RenderTextureのリソース
		Microsoft::WRL::ComPtr<ID3D12Resource> resource_ = nullptr;

		// RTV専用のディスクリプタヒープ
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_ = nullptr;
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_{};

		// シェーダーマネージャーに渡すときに使うインデックス
		uint32_t srvIndex_ = 0;
	};
}