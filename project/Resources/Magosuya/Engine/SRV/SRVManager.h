#pragma once
#include "DxCommon.h"
#include <queue>

class SRVManager {
public:
	static SRVManager* GetInstance() {
		////初めて呼び出されたときに一回だけ初期化
		static SRVManager instance;
		return &instance;
	}

	//初期化
	void Initialize(DxCommon* dxCommon);

	//確保
	uint32_t Allocate();

	//破棄
	void Free(uint32_t index);

	//描画前の処理
	void PreDraw();

	//SRVをDescriptorTableにセット
	void SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex);

	//ディスクリプタハンドル計算
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);

	//ディスクリプタヒープ取得
	ID3D12DescriptorHeap* GetDescriptorHeap() { return descriptorHeap_.Get(); }

	// SRV生成(texture)
	void CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT Format, UINT MipLevels, TextureData texData);
	// SRV生成(Structured Buffer)
	void CreateSRVStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride);
	// SRV生成(RenderTexture)
	void CreateSRVforRenderTexture(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT format);

private:
	//ディスクリプタヒープ作成関数
	void CreateDescriptorHeap();

private:
	//コンストラクタを禁止
	SRVManager() = default;
	// コピーコンストラクタと代入演算子を禁止
	SRVManager(const SRVManager&) = delete;
	SRVManager& operator=(const SRVManager&) = delete;
	SRVManager(SRVManager&&) = delete;
	SRVManager& operator=(SRVManager&&) = delete;

private:
	//SRVの最大数
	static inline const uint32_t kMaxSRVCount_ = 512;
	//SRV用のディスクリプタサイズ
	uint32_t descriptorSize_ = 0;
	//SRV用ディスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> descriptorHeap_;

	//ディスクリプタヒープインデックスの管理
	static inline UINT nextDescriptorIndex_ = 0;;
	std::queue<int> freeIndexQueue_;

	//ポインタを借りる
	DxCommon* dxCommon_ = nullptr;
};