#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include "struct.h" // struct.h をインクルード
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Skeleton.h"
#include "SkinCluster.h"

struct ID3D12Device;

// メッシュ描画用の頂点データ
struct MeshData {
	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;
};

struct MeshResource {
	VertexBuffer<VertexData> vertexBuffer;
	IndexBuffer<uint32_t> indexBuffer;

	// デバイスを受け取って初期化する
	void Initialize(ID3D12Device* device, const MeshData& data) {
		// 第2引数は配列ではなく、サイズ(頂点数)を渡すように修正
		vertexBuffer.Initialize(device, data.vertices.size()); 
		vertexBuffer.Update(data.vertices);

		indexBuffer.Initialize(device, data.indices.size());
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

	// CPU側の階層構造やマテリアル、スキンクラスター用の生データ
	Node rootNode{};
	std::string textureFilePath{};
	std::unordered_map<std::string, JointWeightData> skinClusterData{};
};