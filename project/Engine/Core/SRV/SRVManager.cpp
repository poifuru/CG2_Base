#include "PCH.h"
//#include "SRVManager.h"
//#include <cassert>
//
//void SRVManager::Initialize(DxCommon* dxCommon) {
//	//ポインタの登録
//	dxCommon_ = dxCommon;
//
//	//ディスクリプタヒープの生成
//	CreateDescriptorHeap();
//
//	//ディスクリプタ1個分のサイズを取得して記録
//	descriptorSize_ = dxCommon_->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
//
//	// DSVのSRVを作成する
//	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
//	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
//	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
//	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
//	srvDesc.Texture2D.MipLevels = 1;
//
//	uint32_t index = Allocate();
//
//	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU;
//	handleCPU = GetCPUDescriptorHandle(index);
//
//	dxCommon_->GetDevice()->CreateShaderResourceView(dxCommon_->GetDSV(), &srvDesc, handleCPU);
//}
//
//uint32_t SRVManager::Allocate() {
//	//インデックスが上限に達していないかチェック
//	assert(nextDescriptorIndex_ < kMaxSRVCount_ || !freeIndexQueue_.empty());
//
//	//SRVを作成するDescriptorHeapの場所を決める
//	UINT newIndex;
//
//	//キューの空きリストをチェック
//	if(!freeIndexQueue_.empty()) {
//		//空きがあればそこを使う
//		newIndex = freeIndexQueue_.front();
//		freeIndexQueue_.pop();	//キューから取り除く
//	}
//	else {
//		//空きがなかったら、次に割り当てるインデックスを使う
//		newIndex = nextDescriptorIndex_;
//		nextDescriptorIndex_++;	//線形ポインタを進める
//	}
//
//	return newIndex;
//}
//
//void SRVManager::Free(uint32_t index) {
//	freeIndexQueue_.push(index);
//}
//
//void SRVManager::PreDraw() {
//	//描画用DescriptorHeapの設定
//	ID3D12DescriptorHeap* descriptorHeaps[] = { descriptorHeap_.Get() };
//	dxCommon_->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps);
//}
//
//void SRVManager::SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex) {
//	dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(RootParameterIndex, GetGPUDescriptorHandle(srvIndex));
//}
//
//D3D12_CPU_DESCRIPTOR_HANDLE SRVManager::GetCPUDescriptorHandle(uint32_t index) {
//	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap_->GetCPUDescriptorHandleForHeapStart();
//	handleCPU.ptr += static_cast<UINT64>(descriptorSize_) * static_cast<UINT64>(index);
//	return handleCPU;
//}
//
//D3D12_GPU_DESCRIPTOR_HANDLE SRVManager::GetGPUDescriptorHandle(uint32_t index) {
//	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap_->GetGPUDescriptorHandleForHeapStart();
//	handleGPU.ptr += static_cast<UINT64>(descriptorSize_) * static_cast<UINT64>(index);
//	return handleGPU;
//}
//
//void SRVManager::CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT Format, UINT MipLevels, TextureData texData) {
//	//metaDataをもとにSRVの設定
//	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
//	srvDesc.Format = Format;
//	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
//	// CubeMapである場合の分岐
//	if(texData.metadata.IsCubemap()) {
//		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
//		srvDesc.TextureCube.MostDetailedMip = 0;	// unionがTextureCubeになったが、内部パラメータの意味はTexture2Dと変わらない
//		srvDesc.TextureCube.MipLevels = UINT_MAX;
//		srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
//	}
//	else {
//		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
//		srvDesc.Texture2D.MipLevels = UINT(MipLevels);
//	}
//
//	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU;
//	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU;
//	handleCPU = GetCPUDescriptorHandle(srvIndex);
//	handleGPU = GetGPUDescriptorHandle(srvIndex);
//
//	//実際にSRVを生成
//	dxCommon_->GetDevice()->CreateShaderResourceView(pResource, &srvDesc, handleCPU);
//}
//
//void SRVManager::CreateSRVStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride) {
//	//particle用SRVを作成する
//	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
//	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
//	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
//	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
//	srvDesc.Buffer.FirstElement = 0;
//	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
//	srvDesc.Buffer.NumElements = numElements;
//	srvDesc.Buffer.StructureByteStride = structureByteStride;
//	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = GetCPUDescriptorHandle(srvIndex);
//	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = GetGPUDescriptorHandle(srvIndex);
//	dxCommon_->GetDevice()->CreateShaderResourceView(pResource, &srvDesc, handleCPU);
//}
//
//void SRVManager::CreateSRVforRenderTexture(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT format) {
//	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
//	srvDesc.Format = format;
//	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
//	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 固定で2Dテクスチャとして生成
//	srvDesc.Texture2D.MipLevels = 1;                        // レンダーテクスチャなのでミップは1固定
//	srvDesc.Texture2D.MostDetailedMip = 0;
//	srvDesc.Texture2D.PlaneSlice = 0;
//	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
//
//	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = GetCPUDescriptorHandle(srvIndex);
//
//	// SRVの生成
//	dxCommon_->GetDevice()->CreateShaderResourceView(pResource, &srvDesc, handleCPU);
//}
//
//void SRVManager::CreateDescriptorHeap() {
//	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
//
//	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
//	descriptorHeapDesc.NumDescriptors = kMaxSRVCount_;
//	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
//
//	HRESULT hr = dxCommon_->GetDevice()->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(descriptorHeap_.GetAddressOf()));
//	assert(SUCCEEDED(hr));
//}