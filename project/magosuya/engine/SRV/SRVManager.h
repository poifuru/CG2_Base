#pragma once
#include "DxCommon.h"

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

	//ディスクリプタハンドル計算
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);

	//SRV生成(texture)
	void CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT Format, UINT MipLevels);
	//SRV生成(Structured Buffer)
	void CreateSRVStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride);

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

	//次に使うSRVインデックス
	uint32_t useIndex_ = 0;


	//ポインタを借りる
	DxCommon* dxCommon_ = nullptr;
};