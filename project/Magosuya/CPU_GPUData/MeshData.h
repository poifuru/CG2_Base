#pragma once
#include "Buffer.h"
#include "Skeleton.h"
#include "SkinCluster.h"

// 汎用的な頂点データの構造体
struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

// メッシュ描画用の頂点データ
struct MeshData {
	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;
};

struct MeshResource {
	VertexBuffer<VertexData> vertexBuffer;
	IndexBuffer<uint32_t> indexBuffer;

	// ヘルパー関数を作っておく
	void Initialize(DxCommon* dxCommon, const MeshData& data) {
		vertexBuffer.Initialize(dxCommon, data.vertices);
		vertexBuffer.Update(data.vertices);

		indexBuffer.Initialize(dxCommon, data.indices.size());
		indexBuffer.Update(data.indices);
	}
};

struct ModelData {
	// Assimpで読み込んだ生データの受け皿
	MeshData meshData;

	MeshResource meshResource; // GPU側のVB/IBラッパー

	// シェーダーのドローコールやビューセットに最低限必要な情報
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	D3D12_INDEX_BUFFER_VIEW  ibView{};
	uint32_t vertexCount = 0;
	uint32_t indexCount = 0;

	// CPU側の階層構造やマテリアル、スキンクラスター用の生データ（Assimpで読んだやつ）
	Node rootNode{};
	std::string textureFilePath{};
	std::unordered_map<std::string, JointWeightData> skinClusterData{};

	// メモリ節約のため、GPU転送が終わったらCPU側の配列（vertices/indices）はクリアしてもOK
};