#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <d3d12.h>
#include <wrl.h>
#include "TextureData.h"

class DescriptorHeapManager;

class TextureManager {
public:		//外部公開メソッド
	TextureManager() = default;
	~TextureManager(); // 終了時に残ったテクスチャがあれば解放する

	void Initialize (ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorHeapManager& heapManager);

	//画像をロードする関数
	uint32_t LoadTexture (
		const std::string& filePath,
		const std::string& id,
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		DescriptorHeapManager& heapManager
	);

	// テクスチャアンロード（使い終わったら参照カウントを減らす）
	void UnloadTexture(const std::string& id, DescriptorHeapManager& heapManager);

	//中間リソース解放関数
	void ClearIntermediateResource ();

	// テクスチャIDからバインドレスヒープ内のインデックスを取得する
	uint32_t GetTextureIndex(const std::string& id) const {
		auto it = textureMap_.find(id);
		if (it != textureMap_.end()) {
			return it->second.textureIndex;
		}
		return 0; // 見つからない場合はデフォルトの0番
	}

public:
	// コピー・移動禁止
	TextureManager(const TextureManager&) = delete;
	TextureManager& operator=(const TextureManager&) = delete;
	TextureManager(TextureManager&&) = delete;
	TextureManager& operator=(TextureManager&&) = delete;

private:	//内部関数
	// 内部用ヘルパー関数群
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata);

	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		const Microsoft::WRL::ComPtr<ID3D12Resource>& texture,
		const DirectX::ScratchImage& mipImages
	);

private:	//メンバ変数
	// テクスチャID（"Player"など）と実体データのマップ
	std::unordered_map<std::string, TextureData> textureMap_;

	// GPUにデータをコピーする間だけ生きている必要がある一時的なバッファのゴミ箱
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> intermediateResources_;
};

