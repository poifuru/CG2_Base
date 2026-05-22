#pragma once
#include <cstdint>

#include "PSOManager.h"

static const size_t kMaxRootParameters = 16;

// レジスタにバインドするリソースの種類
enum class BindingType : UINT {
	None,
	CBV,
	SRV_Table,	// ディスクリプタテーブル
};

// バインド情報をまとめる構造体
struct RenderBind {
	BindingType type = BindingType::None;
	// unionにすることでどちらか１つ分の変数のメモリしか持たなくなる(共有してる)のでメモリの節約になる
	// メモリを共有しているだけなのでちゃんと最後に書き込んだ変数にアクセスするようにしないとクラッシュする
	union {
		D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;   // CBV用
		D3D12_GPU_DESCRIPTOR_HANDLE descriptorHandle; // SRVテーブル用
	};
};

// ドローコール必要な情報を完結させるための構造体
struct RenderCommand {
	// どのパイプラインで描画するか
	uint32_t rootSignatureID = 0;
	PSODescriptor psoDesc{};

	// どのメッシュか
	D3D12_VERTEX_BUFFER_VIEW vbViews[2]{};	// 0:頂点 1:スキニング影響度(optional)
	D3D12_INDEX_BUFFER_VIEW ibv{};
	UINT indexCount = 0;

	// === ルートパラメータごとのバインドデータ ===
	// ルートパラメータのインデックス（0〜15）にダイレクトに対応させる
	RenderBind binds[kMaxRootParameters]{};

	// 描画順ソート用のプライオリティ(不透明:0、透明:1など)
	uint8_t layer = 0;
};