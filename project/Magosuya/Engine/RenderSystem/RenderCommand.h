#pragma once
#include <optional>

#include "PSOManager.h"

// ドローコール必要な情報を完結させるための構造体
struct RenderCommand {
	// どのパイプラインで描画するか
	uint32_t rootSignatureID = 0;
	PSODescriptor psoDesc{};

	// どのメッシュか
	D3D12_VERTEX_BUFFER_VIEW vbViews[2]{};	// 0:頂点 1:スキニング影響度(optional)
	D3D12_INDEX_BUFFER_VIEW ibv{};
	UINT indexCount = 0;

	// どの定数バッファやSRVを使うか	(RootParameterのインデックス順に並べる)
	D3D12_GPU_VIRTUAL_ADDRESS transformCBV = 0;	// パラメータ0
	D3D12_GPU_VIRTUAL_ADDRESS materialCBV = 0;	// パラメータ1
	D3D12_GPU_DESCRIPTOR_HANDLE textureSRV{};    // パラメータ4
	D3D12_GPU_DESCRIPTOR_HANDLE skinningSRV{};   // パラメータ9 (スキニング用)

	// 描画順ソート用のプライオリティ(不透明:0、透明:1など)
	uint8_t layer = 0;
};

// ドローコール必要な情報を完結させるための構造体
struct InstanceRenderCommand {
	// どのパイプラインで描画するか
	uint32_t rootSignatureID = 0;
	PSODescriptor psoDesc{};

	// どのメッシュか（全員共通の見た目）
	D3D12_VERTEX_BUFFER_VIEW vbViews[2]{};	// 0:頂点 1:スキニング影響度(基本0でOK)
	D3D12_INDEX_BUFFER_VIEW ibv{};
	UINT indexCount = 0;

	// === [ここが重要！] 大量のトランスフォームが入ったStructuredBufferのSRVハンドル ===
	// パラメータ0に ConstantBuffer ではなく StructuredBuffer(SRV) をバインドする
	D3D12_GPU_DESCRIPTOR_HANDLE transformsSRV{};

	// 共通のマテリアル（全員同じ色・質感・テクスチャになる）
	D3D12_GPU_VIRTUAL_ADDRESS materialCBV = 0;	// パラメータ1
	D3D12_GPU_DESCRIPTOR_HANDLE textureSRV{};    // パラメータ4

	// === 何個描画するか ===
	UINT instanceCount = 0;

	// 描画順ソート用のプライオリティ
	uint8_t layer = 0;
};