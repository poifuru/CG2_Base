#include "RenderTexture.h"
#include "SRVManager.h"
#include "WindowsAPI.h"
#include "DxCommon.h"

RenderTexture::RenderTexture() {
}

RenderTexture::~RenderTexture() {
}

void RenderTexture::Initialize(DxCommon* dxCommon, SRVManager* srvManager) {
	// オフスクリーンレンダリング用のクリアカラー（わかりやすいように少し暗い色など自由に変更可能）
	const Vector4 kRenderTargetClearValue{ 0.0f, 0.1f, 0.2f, 1.0f }; // DxCommonのClear色と同じ

	// リソース作成
	CreateRenderTextureResource(
		dxCommon->GetDevice(),
		WindowsAPI::GetInstance()->kClientWidth,
		WindowsAPI::GetInstance()->kClientHeight,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		kRenderTargetClearValue
	);

	// RTV作成
	rtvHandle_ = dxCommon->AllocateRTV();
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	dxCommon->GetDevice()->CreateRenderTargetView(resource_.Get(), &rtvDesc, rtvHandle_);

	TextureData texData = {};

	// SRV作成
	srvIndex_ = srvManager->Allocate();
	srvManager->CreateSRVforRenderTexture(srvIndex_, resource_.Get(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
}

ComPtr<ID3D12Resource> RenderTexture::CreateRenderTextureResource(ComPtr<ID3D12Device> device, uint32_t width, uint32_t height, DXGI_FORMAT format, const Vector4& clearColor) {
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
	device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,	// 最初はSRV(PIXEL_SHADER_RESOURCE)として作っておく
		&clearValue,	// Clear最適値。ClearRenderTargetをこの色でClearするようにする。最適化されているので高速である。
		IID_PPV_ARGS(&resource_)
	);

	// 作ったresourceを返す
	return resource_;
}
