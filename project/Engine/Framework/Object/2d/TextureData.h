#pragma once

struct TextureData {
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource;
	DirectX::TexMetadata metadata;
	uint32_t textureIndex = 0; // バインドレスヒープ内の配列番号
	uint32_t refCount = 0;     // 参照カウンタ
};