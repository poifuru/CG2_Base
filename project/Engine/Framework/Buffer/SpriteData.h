#pragma once
#include "struct.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"

static const uint32_t kSpriteIndexNum = 6;

// スプライト専用頂点データ
struct SpriteVertexData {
	Vector4 position;
	Vector2 texcoord;
};

// スプライト専用行列データ
struct SpriteMatrixData {
	Matrix4x4 wvp;	// 2Dカメラにも対応できるようにviewは含めておく
};

// スプライトのマテリアルデータ
struct SpriteMaterialData {
	Vector4 color;
	Matrix4x4 uvTransform;
};

// Sprite構造体
struct SpriteData {
	Vector2 position;
	Vector2 scale;
	float rotation;
	Vector2 size;
	Vector2 anchorPoint;
	bool flipX = false;
	bool flipY = false;
	std::string textureName;
};

struct SpriteResource {
	VertexBuffer<SpriteVertexData> vertexBuffer;
	IndexBuffer<uint32_t> indexBuffer;
	ConstantBuffer<SpriteMatrixData> matrixBuffer;
	ConstantBuffer<SpriteMaterialData> materialBuffer;

	void Initialize(ID3D12Device* device) {
		// 頂点データの初期化
		std::vector<SpriteVertexData> vertices(4);

		// 左側が 0.0f 右側が 1.0f
		vertices[0].position = { 0.0f, 0.0f, 0.0f, 1.0f };	// 左上
		vertices[0].texcoord = { 0.0f, 0.0f };

		vertices[1].position = { 1.0f, 0.0f, 0.0f, 1.0f };	// 右上
		vertices[1].texcoord = { 1.0f, 0.0f };

		vertices[2].position = { 0.0f, 1.0f, 0.0f, 1.0f };	// 左下
		vertices[2].texcoord = { 0.0f, 1.0f };

		vertices[3].position = { 1.0f, 1.0f, 0.0f, 1.0f };	// 右下
		vertices[3].texcoord = { 1.0f, 1.0f };

		vertexBuffer.Initialize(device, vertices);
		vertexBuffer.Update(vertices);	// 初期化時に一回だけ送る

		std::vector<uint32_t> indices(6);

		indices[0] = 0;
		indices[1] = 1;
		indices[2] = 2;

		indices[3] = 1;
		indices[4] = 3;
		indices[5] = 2;

		indexBuffer.Initialize(device, kSpriteIndexNum);
		indexBuffer.Update(indices);

		matrixBuffer.Initialize(device);
		materialBuffer.Initialize(device);
	}

	void Update(const SpriteMatrixData& matrixData, const SpriteMaterialData& materialData) {
		matrixBuffer.Update(matrixData);
		materialBuffer.Update(materialData);
	}
};