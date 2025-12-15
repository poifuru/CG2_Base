#include "SRVManager.h"
#include <cassert>

void SRVManager::Initialize(DxCommon* dxCommon) {
	//ポインタの登録
	dxCommon_ = dxCommon;

	//ディスクリプタヒープの生成
	descriptorHeap_ = dxCommon_->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMaxSRVCount_, true);

	//ディスクリプタ1個分のサイズを取得して記録
	descriptorSize_ = dxCommon_->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

uint32_t SRVManager::Allocate() {
	//インデックスが上限に達していないかチェック
	assert(useIndex_ != kMaxSRVCount_);

	//returnする番号を一旦記録しておく
	int index = useIndex_;
	//次回のために番号を1進める
	useIndex_++;
	//記録した番号を返す
	return index;
}

D3D12_CPU_DESCRIPTOR_HANDLE SRVManager::GetCPUDescriptorHandle(uint32_t index) {
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize_ * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE SRVManager::GetGPUDescriptorHandle(uint32_t index) {
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap_->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize_ * index);
	return handleGPU;
}

void SRVManager::CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT Format, UINT MipLevels) {

}

void SRVManager::CreateSRVStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride) {
	//particle用SRVを作成する
	D3D12_SHADER_RESOURCE_VIEW_DESC StructuredDesc = {};
	StructuredDesc.Format = DXGI_FORMAT_UNKNOWN;
	StructuredDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	StructuredDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	StructuredDesc.Buffer.FirstElement = 0;
	StructuredDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	StructuredDesc.Buffer.NumElements = numElements;
	StructuredDesc.Buffer.StructureByteStride = structureByteStride;
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = GetCPUDescriptorHandle(srvIndex);
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = GetGPUDescriptorHandle(srvIndex);
	dxCommon_->GetDevice()->CreateShaderResourceView(pResource, &StructuredDesc, handleCPU);
}
