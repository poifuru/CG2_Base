#include "PCH.h"
#include "TextureManager.h"
#include "DescriptorHeapManager.h"
#include "ChangeString.h"

TextureManager::~TextureManager() {
	// 念のためテクスチャマップを空にする
	textureMap_.clear();
	intermediateResources_.clear();
}

void TextureManager::Initialize (ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorHeapManager* heapManager) {
	device_ = device;
	cmdList_ = cmdList;
	heapManager_ = heapManager;

	// 何も読み込まれなかった時用の真っ白なダミーテクスチャを登録しておく
	// RGBAの1x1ピクセルの画像をメモリ上で即席で作る
	DirectX::ScratchImage image;
	HRESULT hr = image.Initialize2D(DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 1, 1);
	assert(SUCCEEDED(hr));

	uint32_t* pixelData = reinterpret_cast<uint32_t*>(image.GetPixels());
	*pixelData = 0xFFFFFFFF; // 白

	TextureData dummyData{};
	dummyData.refCount = 1;
	dummyData.metadata = image.GetMetadata();
	dummyData.textureResource = CreateTextureResource(device_, dummyData.metadata);

	// コピー用のコマンドを積む
	intermediateResources_.push_back(UploadTextureData(device_, cmdList_, dummyData.textureResource, image));

	// バインドレスヒープからダミー用の枠を1つ確保
	dummyData.textureIndex = heapManager_->AllocateIndex();

	// SRVの設定をして焼き付ける
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = dummyData.metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	heapManager_->CreateSRVforTexture2D(dummyData.textureIndex, dummyData.textureResource.Get(), srvDesc);

	// "white1x1" という名前で本棚に保管
	textureMap_["white1x1"] = dummyData;
}

uint32_t TextureManager::LoadTexture (const std::string& filePath) {
	// すでに同じパスで読み込まれていたら、新しく作らずに参照カウントだけ増やしてインデックスを返す
	if (textureMap_.count(filePath)) {
		textureMap_.at(filePath).refCount++;
		return textureMap_.at(filePath).textureIndex;
	}

	HRESULT hr = S_OK;
	DirectX::ScratchImage image;

	//実際に読み込む処理
	//テクスチャファイルを読み込んでプログラムで扱えるようにする
	std::wstring filePathW = String::ConvertString (filePath);
	OutputDebugStringW ((L"探してるファイル: " + filePathW + L"\n").c_str ());

	// 拡張子を見てDDSかそれ以外（WIC）か自動で切り替える
	if (filePathW.ends_with(L".dds")) {
		hr = DirectX::LoadFromDDSFile(filePathW.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
	} else {
		hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	}

	// 読み込みに失敗したらログを出して、安全のためにダミーテクスチャの番号を返す
	if (FAILED(hr)) {
		OutputDebugStringW((L"[Warning] テクスチャのロードに失敗しました: " + filePathW + L"\n").c_str());

		// ダミーの参照を1個増やして、ダミーのインデックスを返す
		textureMap_.at("white1x1").refCount++;
		return textureMap_.at("white1x1").textureIndex;
	}

	// ミップマップの自動生成
	DirectX::ScratchImage mipImage;
	if (DirectX::IsCompressed(image.GetMetadata().format)) {
		mipImage = std::move(image);
	} else {
		hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 4, mipImage);
		if (FAILED(hr)) {
			mipImage = std::move(image); // 失敗したら元の画像で妥協
		}
	}

	TextureData newData{};
	newData.refCount = 1;
	newData.metadata = mipImage.GetMetadata();
	newData.textureResource = CreateTextureResource(device_, newData.metadata);

	// VRAMに転送するコマンドを記録（中間リソースをゴミ箱に積む）
	intermediateResources_.push_back(UploadTextureData(device_, cmdList_, newData.textureResource, mipImage));

	// バインドレスヒープのインデックスを1つ切り出す
	newData.textureIndex = heapManager_->AllocateIndex();

	// 案内札（SRV）の記述を組み立てる
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = newData.metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = static_cast<UINT>(newData.metadata.mipLevels);

	// ヒープマネージャーに、指定されたインデックス位置へSRVを生成してもらう
	heapManager_->CreateSRVforTexture2D(newData.textureIndex, newData.textureResource.Get(), srvDesc);

	// マップに登録して、割り当てられたインデックス番号を返す
	textureMap_[filePath] = newData;
	return newData.textureIndex;
}

void TextureManager::UnloadTexture(const std::string& filePath) {
	if (!textureMap_.count(filePath)) return;

	TextureData& data = textureMap_.at(filePath);
	data.refCount--;

	// 誰も使わなくなったら完全にメモリから削除する
	if (data.refCount <= 0) {
		// 使い終わったインデックスを巨大ヒープの空きリストに返却する
		heapManager_->FreeIndex(data.textureIndex);

		// キャッシュマップから削除
		textureMap_.erase(filePath);
	}
}

void TextureManager::ClearIntermediateResource () {
	// コピーが完全に終わった中間リソースを一斉掃除する
	intermediateResources_.clear();
}

Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata) {
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = static_cast<UINT>(metadata.width);
	resourceDesc.Height = static_cast<UINT>(metadata.height);
	resourceDesc.MipLevels = static_cast<UINT>(metadata.mipLevels);
	resourceDesc.DepthOrArraySize = static_cast<UINT>(metadata.arraySize);
	resourceDesc.Format = metadata.format;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // 最速のビデオメモリ領域

	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_COPY_DEST, // まずはデータ転送を受ける状態
		nullptr,
		IID_PPV_ARGS(resource.GetAddressOf())
	);
	assert(SUCCEEDED(hr));
	return resource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::UploadTextureData(
	ID3D12Device* device,
	ID3D12GraphicsCommandList* cmdList,
	const Microsoft::WRL::ComPtr<ID3D12Resource>& texture,
	const DirectX::ScratchImage& mipImages
) {
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	DirectX::PrepareUpload(device, mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresources);

	uint64_t intermediateSize = GetRequiredIntermediateSize(texture.Get(), 0, static_cast<UINT>(subresources.size()));

	// アップロード用の中間バッファを自前で作成
	D3D12_HEAP_PROPERTIES uploadHeapProps{};
	uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC bufferDesc{};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Width = intermediateSize;
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource;
	HRESULT hr = device->CreateCommittedResource(
		&uploadHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(intermediateResource.GetAddressOf())
	);
	assert(SUCCEEDED(hr));

	// コマンドリストにデータ転送を記録
	UpdateSubresources(cmdList, texture.Get(), intermediateResource.Get(), 0, 0, static_cast<UINT>(subresources.size()), subresources.data());

	// 転送が終わったらシェーダーから読める状態（GENERIC_READ）にバリアを張る
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = texture.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	cmdList->ResourceBarrier(1, &barrier);

	return intermediateResource;
}