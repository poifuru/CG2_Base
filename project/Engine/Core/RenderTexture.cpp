#include "PCH.h"
#include "RenderTexture.h"
#include "DescriptorHeapManager.h"
#include "WindowsAPI.h"
#include <cassert>

void RenderTexture::Initialize(ID3D12Device* device, DescriptorHeapManager* heapManager) {
	// オフスクリーンレンダリング用のクリアカラー
	const Vector4 kRenderTargetClearValue{ 0.1f, 0.25f, 0.5f, 1.0f }; // SwapChainのClear色と合わせる

	// リソース作成
	CreateRenderTextureResource(
		device,
		WindowsAPI::GetInstance()->GetWindowWidth(),
		WindowsAPI::GetInstance()->GetWindowHeight(),
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		kRenderTargetClearValue
	);

	// RTV用のヒープを1つだけ作成
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.NumDescriptors = 1;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	HRESULT hr = device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap_));
	assert(SUCCEEDED(hr));

	rtvHandle_ = rtvHeap_->GetCPUDescriptorHandleForHeapStart();

	// RTV作成
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	device->CreateRenderTargetView(resource_.Get(), &rtvDesc, rtvHandle_);

	// SRV作成
	srvIndex_ = heapManager->AllocateIndex();
	
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	heapManager->CreateSRVforTexture2D(srvIndex_, resource_.Get(), srvDesc);
}

Microsoft::WRL::ComPtr<ID3D12Resource> RenderTexture::CreateRenderTextureResource(ID3D12Device* device, uint32_t width, uint32_t height, DXGI_FORMAT format, const Vector4& clearColor) {
	// Resourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = format;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	// ヒープの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.VisibleNodeMask = 1;

	// クリアカラーを設定
	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = format;
	clearValue.Color[0] = clearColor.x;
	clearValue.Color[1] = clearColor.y;
	clearValue.Color[2] = clearColor.z;
	clearValue.Color[3] = clearColor.w;

	// Resourceの生成
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,	// 最初はSRV(PIXEL_SHADER_RESOURCE)として作っておく
		&clearValue,	// Clear最適値。ClearRenderTargetをこの色でClearするようにする。
		IID_PPV_ARGS(&resource_)
	);
	assert(SUCCEEDED(hr));

	// 作ったresourceを返す
	return resource_;
}
