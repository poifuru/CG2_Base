#pragma once
#include "DxCommon.h"

class RenderTexture {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	RenderTexture();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~RenderTexture();

	/// <summary>
	/// RenderTexture生成関数
	/// </summary>
	/// <param name="device">デバイス</param>
	/// <param name="width">幅</param>
	/// <param name="height">高さ</param>
	/// <param name="format">フォーマット</param>
	/// <param name="clearColor">クリアカラー</param>
	/// <returns>RenderTexture</returns>
	ComPtr<ID3D12Resource> CreateRenderTextureResource(
		ComPtr<ID3D12Device> device, uint32_t width, uint32_t height, DXGI_FORMAT format, const Vector4& clearColor
	);

	// アクセッサ
	ID3D12Resource* GetResource() { return resource_.Get(); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle() { return rtvHandle_; }
	uint32_t GetSrvIndex() { return srvIndex_; }

private:
	// RenderTextureのリソース
	ComPtr<ID3D12Resource> resource_ = nullptr;

	// RTVにセットするためのハンドル
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_{};

	// シェーダーマネージャーに渡すときに使うインデックス
	uint32_t srvIndex_ = 0;
};