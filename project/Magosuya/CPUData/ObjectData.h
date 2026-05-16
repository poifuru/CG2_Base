#pragma once
#include "Buffer.h"

// 汎用的な頂点データの構造体
struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

// オブジェクト描画用の頂点データ
struct ObjectData {
	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;
};

struct ObjectResource {
	VertexBuffer<VertexData> vertexBuffer;
	IndexBuffer<uint32_t> indexBuffer;

	// ヘルパー関数を作っておく
	void Initialize(DxCommon* dxCommon, const ObjectData& data) {
		vertexBuffer.Initialize(dxCommon, data.vertices);
		vertexBuffer.Update(data.vertices);

		indexBuffer.Initialize(dxCommon, data.indices.size());
		indexBuffer.Update(data.indices);
	}
};