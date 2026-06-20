#pragma once
#include <cstdint>
#include <d3d12.h>
#include "PSOManager.h"

// 最初のゴール：マテリアル付き三角形のための最小限の構造体
struct RenderCommand {
	PSODescriptor psoDesc{};       // どのパイプラインで描画するか（VS/PSのIDなどが入る）

	// メッシュ情報（三角形）
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	D3D12_INDEX_BUFFER_VIEW ibv{};
	UINT indexCount = 0;

	// ★バインドレスの核心：ハメ換えるリソースポインタの代わりに、ただの「インデックス（数値）」を持つ！
	uint32_t materialIndex = 0;   // 構造化バッファ（マテリアル配列）の何番目か
	uint32_t textureIndex = 0;    // ディスクリプタヒープの何番目にテクスチャがあるか

	// オブジェクト個別のトランスフォームバッファのGPU仮想アドレス
	D3D12_GPU_VIRTUAL_ADDRESS transformGPUAddress = 0;

	uint8_t layer = 0;            // 不透明：0、半透明：1 などのソート用
};