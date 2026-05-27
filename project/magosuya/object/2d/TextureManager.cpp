#include "TextureManager.h"
#include <d3dx12.h>
#include <DirectXTex.h>
#include <filesystem>
#include "ChangeString.h"

void TextureManager::Initialize (DxCommon* dxCommon) {
	dxCommon_ = dxCommon;
	srvManager_ = SRVManager::GetInstance();

	// ダミーの生成（戻り値のインデックスを記録しておく）
	dummyTextureIndex_ = CreateDummyTexture();
	dummyCubeTextureIndex_ = CreateDummyCubeTexture();
}

int TextureManager::LoadTexture (const std::string& filePath) {
	//そのパスの画像をすでに読み込んでいたら
	if (pathMap_.count (filePath)) {
		//既存データを取得
		int existingIndex = pathMap_.at (filePath);
		//参照カウントを増やす
		textureMap_.at(existingIndex).ref_count++;
		//存在していたら既存のデータを返す
		return existingIndex;
	}

	HRESULT hr;

	//returnするデータを詰める箱
	TextureData newData{};
	newData.ref_count = 1;

	//実際に読み込む処理
	//テクスチャファイルを読み込んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = String::ConvertString (filePath);
	OutputDebugStringW ((L"探してるファイル: " + filePathW + L"\n").c_str ());

	// ファイルパスの拡張子で分岐
	if(filePathW.ends_with(L".dds")) {
		hr = DirectX::LoadFromDDSFile(filePathW.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
	}
	else {
		hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	}

	if (FAILED (hr)) {
		std::wstringstream ss;
		ss << L"[エラー] テクスチャ読み込み失敗！ ダミーのテクスチャを返します HRESULT: 0x" << std::hex << hr << std::endl;
		OutputDebugStringW (ss.str ().c_str ());

		// パスに "dds" が含まれる、あるいは特定の条件ならキューブ用のダミーを返す
		if (filePath.find(".dds") != std::string::npos) { // 簡易的な判定
			textureMap_.at(dummyCubeTextureIndex_).ref_count++;
			pathMap_[filePath] = dummyCubeTextureIndex_;
			return dummyCubeTextureIndex_;
		}

		// ダミーテクスチャのインデックスの参照カウントを増やして返す
		textureMap_.at(dummyTextureIndex_).ref_count++;
		// 失敗したパスもダミーを指すようにキャッシュしておく
		pathMap_[filePath] = dummyTextureIndex_;
		return dummyTextureIndex_;
	}
	assert (SUCCEEDED (hr));

	//ミップマップの作成
	DirectX::ScratchImage mipImage{};
	if(DirectX::IsCompressed(image.GetMetadata().format)) {
		mipImage = std::move(image);
	}
	else {
		hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 4, mipImage);
	}

	assert (SUCCEEDED (hr));

	//mipImageを使ってmetaDataを作る
	newData.metadata = mipImage.GetMetadata ();
	//作ったmetaDataをもとにテクスチャリソースを作成
	newData.textureResource = CreateTextureResource (newData.metadata);
	//実際にデータを転送
	intermediateResource_.push_back(UploadTextureData (newData.textureResource, mipImage));

	//srvManagerで空きディスクリプタのインデックスを確保
	UINT newIndex = srvManager_->Allocate();
	int textureID = static_cast<int>(newIndex);

	//srvManagerでSRVの生成
	srvManager_->CreateSRVforTexture2D(
		newIndex,
		newData.textureResource.Get(),
		newData.metadata.format,
		(UINT)newData.metadata.mipLevels,
		newData.metadata
	);

	//生成物をmapに渡すためにデータを詰める
	newData.handle.gpu = srvManager_->GetGPUDescriptorHandle(newIndex);
	//どのインデックスを使ったかを保存しておくと解放時に便利
	newData.descriptorIndex = newIndex;

	// 両方のマップに登録
	textureMap_[textureID] = newData;
	pathMap_[filePath] = textureID;

	return textureID;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetTextureHandle (int textureIndex) {
	if (textureMap_.count(textureIndex) == 0) {
		return textureMap_.at(dummyTextureIndex_).handle.gpu;
	}
	return textureMap_.at(textureIndex).handle.gpu;
}

void TextureManager::UnloadTexture (int textureIndex) {
	//参照カウントを減らす
	if (!textureMap_.count (textureIndex)) { return; }
	textureMap_.at (textureIndex).ref_count--;

	// 参照を取得する
	TextureData& data = textureMap_.at (textureIndex);

	//参照カウントがゼロになったらテクスチャ削除
	if (data.ref_count <= 0) {
		//GPUリソースはComPtrで自動開放
		//使っていたインデックスを空きリストに戻す
		srvManager_->Free(data.descriptorIndex);
		//キャッシュマップからデータを削除
		textureMap_.erase (textureIndex);
	}
}

std::string TextureManager::GetTexturePath(int textureIndex) {
	if (textureMap_.count(textureIndex) == 0) return "";
	return textureMap_.at(textureIndex).filePath;
}

void TextureManager::ClearIntermediateResource () {
	intermediateResource_.clear ();
}

const DirectX::TexMetadata& TextureManager::GetMetaData (int textureIndex) {
	assert (textureMap_.count (textureIndex));

	return textureMap_.at (textureIndex).metadata;
}

ComPtr<ID3D12Resource> TextureManager::CreateTextureResource (const DirectX::TexMetadata& metadata) {
	//1.metadataを基にResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT (metadata.width);								//Textureの幅
	resourceDesc.Height = UINT (metadata.height);							//Textureの高さ
	resourceDesc.MipLevels = UINT (metadata.mipLevels);						//mipmapの数
	resourceDesc.DepthOrArraySize = UINT (metadata.arraySize);				//奥行　or　配列Textureの配列数
	resourceDesc.Format = metadata.format;									//TextureのFormat
	resourceDesc.SampleDesc.Count = 1;										//サンプリングカウント。1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION (metadata.dimension);	//Textureの次元数。普段使っているのは2次元

	//2.利用するHeapの設定。非常に特殊な運用。
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;			//細かい設定を行う

	//3.Resourceを生成する
	ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = dxCommon_->GetDevice ()->CreateCommittedResource (
		&heapProperties,					//Heapの設定
		D3D12_HEAP_FLAG_NONE,				//Heapの特殊な設定。特になし。
		&resourceDesc,						//Resourceの設定
		D3D12_RESOURCE_STATE_COPY_DEST,		//データ転送される設定
		nullptr,							//Clear最適値。使わないのでnullptr。
		IID_PPV_ARGS (resource.GetAddressOf ()));			//作成するResourceポインタへのポインタ
	assert (SUCCEEDED (hr));
	return resource;
}

ComPtr<ID3D12Resource> TextureManager::UploadTextureData (const ComPtr<ID3D12Resource>& texture, const DirectX::ScratchImage& mipImages) {
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	DirectX::PrepareUpload (dxCommon_->GetDevice (), mipImages.GetImages (), mipImages.GetImageCount (), mipImages.GetMetadata (), subresources);
	uint64_t intermediateSize = GetRequiredIntermediateSize (texture.Get (), 0, UINT (subresources.size ()));
	ComPtr<ID3D12Resource> intermediateResource = dxCommon_->CreateBufferResource (intermediateSize);
	UpdateSubresources (dxCommon_->GetCommandList (), texture.Get (), intermediateResource.Get (), 0, 0, UINT (subresources.size ()), subresources.data ());

	//Textureへの転送後は利用できるよう、D3D12_RESOURCE_STATE_COPY_DESTからD3D12_RESOURCE_STATE_GENERIC_READへResourceStateを変更する
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture.Get ();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	dxCommon_->GetCommandList ()->ResourceBarrier (1, &barrier);
	return intermediateResource;
}

int TextureManager::CreateDummyTexture () {
	//returnするデータを詰める箱
	TextureData newData{};
	newData.ref_count = 1;

	HRESULT hr;

	//真っ白な1x1ピクセルのDirectX::ScratchImageを作る
	DirectX::ScratchImage image{};

	//テクスチャの初期化とデータの設定
	//RGBA,8bit(32bit)フォーマットで1x1の画像として初期化
	const size_t width = 1;
	const size_t height = 1;
	const DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM; //一般的なフォーマット

	//画像を初期化
	hr = image.Initialize2D (format, width, height, 1, 1);
	assert (SUCCEEDED (hr));

	//ピクセルデータポインタを取得
	uint32_t* pixelData = reinterpret_cast<uint32_t*>(image.GetPixels ());
	// 真っ白な色を設定
	*pixelData = 0xFFFFFFFF;

	//ミップマップの作成
	DirectX::ScratchImage& mipImage = image;

	//mipImageを使ってmetaDataを作る
	newData.metadata = mipImage.GetMetadata ();
	//作ったmetaDataをもとにテクスチャリソースを作成
	newData.textureResource = CreateTextureResource (newData.metadata);
	//実際にデータを転送
	intermediateResource_.push_back (UploadTextureData (newData.textureResource, mipImage));

	//metaDataをもとにSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = newData.metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT (newData.metadata.mipLevels);

	//SRVを作成するDescriptorHeapの場所を決める
	UINT newIndex = srvManager_->Allocate();

	//srvManagerでSRVの生成
	srvManager_->CreateSRVforTexture2D(
		newIndex,
		newData.textureResource.Get(),
		newData.metadata.format,
		(UINT)newData.metadata.mipLevels,
		newData.metadata
	);

	//生成物をmapに渡すためにデータを詰める
	newData.handle.gpu = srvManager_->GetGPUDescriptorHandle(newIndex);
	//どのインデックスを使ったかを保存しておくと解放時に便利
	newData.descriptorIndex = newIndex;

	// キーを割り当てられたインデックス（int）にする
	int textureID = static_cast<int>(newIndex);
	textureMap_[textureID] = newData;

	// パスからの逆引きマップにも登録しておく（"Dummy" というパスで呼ばれたらこれを返す）
	pathMap_["Dummy"] = textureID;

	// 確定した整数IDを返す
	return textureID;
}

int TextureManager::CreateDummyCubeTexture() {
	TextureData newData{};
	newData.ref_count = 1;
	newData.filePath = "DummyCube";

	HRESULT hr;
	DirectX::ScratchImage image{};

	// ★ポイント：arraySize を 6 にしてキューブマップとして初期化する！
	const size_t width = 1;
	const size_t height = 1;
	const size_t depthOrArraySize = 6; // 6面分
	const DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

	hr = image.InitializeCube(format, width, height, 1, 1); // Initialize2D の代わりに Cube を使用
	assert(SUCCEEDED(hr));

	// 6面全てを真っ白（あるいはデバッグ用に目立つ色）に塗りつぶす
	for (size_t i = 0; i < 6; ++i) {
		uint32_t* pixelData = reinterpret_cast<uint32_t*>(image.GetImage(0, i, 0)->pixels);
		*pixelData = 0xFFFFFFFF; // 真っ白
	}

	newData.metadata = image.GetMetadata();
	newData.textureResource = CreateTextureResource(newData.metadata);
	intermediateResource_.push_back(UploadTextureData(newData.textureResource, image));

	UINT newIndex = srvManager_->Allocate();

	// ★SRV生成時、内部の srvManager_->CreateSRVforTexture2D が
	// metadata.miscFlags & D3D12_RESOURCE_MISC_FLAG_TEXTURECUBE を見て
	// 自動で CUBEMAP 用の SRV を作ってくれる設定になっているか確認してね！
	srvManager_->CreateSRVforTexture2D(
		newIndex,
		newData.textureResource.Get(),
		newData.metadata.format,
		(UINT)newData.metadata.mipLevels,
		newData.metadata
	);

	newData.handle.gpu = srvManager_->GetGPUDescriptorHandle(newIndex);
	newData.descriptorIndex = newIndex;

	int textureID = static_cast<int>(newIndex);
	textureMap_[textureID] = newData;
	pathMap_["DummyCube"] = textureID;

	return textureID;
}