#pragma once
#include <Windows.h>
#include <wrl.h>
using namespace Microsoft::WRL;
#include <unordered_map>
#include <string>
#include <vector>
#include "DxCommon.h"
#include "DescriptorHandle.h"
#include "SRVManager.h"

// テクスチャデータ構造体
struct TextureData {
	ComPtr<ID3D12Resource> textureResource;
	DescriptorHandle handle;	// テクスチャリソースハンドル
	DirectX::TexMetadata metadata = {};	// メタデータ
	UINT descriptorIndex = 0;	// どのディスクリプタヒープを使ったか
	int ref_count = 0;	// 参照カウント
	std::string filePath = "";
};

class TextureManager {
public:		//外部公開メソッド
	static TextureManager* GetInstance () {
		////初めて呼び出されたときに一回だけ初期化
		static TextureManager instance;
		return &instance;
	}

	void Initialize (DxCommon* dxCommon);

	//画像をロードする関数
	int LoadTexture (const std::string& filePath);

	//登録した画像のGPUハンドルを取得する関数
	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureHandle (int textureIndex);

	//テクスチャのアンロード関数
	void UnloadTexture (int textureIndex);

	// インデックスからファイルパスを逆引きするための関数
	std::string GetTexturePath(int textureIndex);

	//中間リソース解放関数
	void ClearIntermediateResource ();

	//アクセッサ
	const std::vector<ComPtr<ID3D12Resource>>& GetIntermediateResource () const { return intermediateResource_; }
	const DirectX::TexMetadata& GetMetaData (int textureIndex);

private:
	//コンストラクタを禁止
	TextureManager () = default;
	// コピーコンストラクタと代入演算子を禁止
	TextureManager (const TextureManager&) = delete;
	TextureManager& operator=(const TextureManager&) = delete;
	TextureManager (TextureManager&&) = delete;
	TextureManager& operator=(TextureManager&&) = delete;

private:	//内部関数
	//LoadTextureのヘルパー関数
	//DirectXのTexrureResourceを作る関数
	ComPtr<ID3D12Resource> CreateTextureResource (const DirectX::TexMetadata& metadata);
	//TextureResourceにデータを転送する関数
	[[nodiscard]]
	ComPtr<ID3D12Resource> UploadTextureData (const ComPtr<ID3D12Resource>& texture, const DirectX::ScratchImage& mipImages);

	//ダミーのテクスチャを作成する関数
	int CreateDummyTexture ();

private:	//メンバ変数
	std::unordered_map<int, TextureData> textureMap_;

	// 二重ロード防止用の「パス → インデックス」の検索マップ
	std::unordered_map<std::string, int> pathMap_;

	// ダミーのインデックスを保持しておく
	int dummyTextureIndex_ = 0;

	//中間リソースの解放待ちリスト
	std::vector<ComPtr<ID3D12Resource>> intermediateResource_;

	//ポインタ借りてくる
	DxCommon* dxCommon_ = nullptr;
	SRVManager* srvManager_ = nullptr;
};

