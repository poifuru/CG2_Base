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
		vertexBuffer.Initialize(device, data.vertices.size()); 
		vertexBuffer.Update(data.vertices);

		indexBuffer.Initialize(device, data.indices.size());
		indexBuffer.Update(data.indices);
	}
};

// 単一メッシュの描画用データ構造
struct Mesh {
	MeshData meshData;
	MeshResource meshResource;

	D3D12_VERTEX_BUFFER_VIEW vbView{};
	D3D12_INDEX_BUFFER_VIEW  ibView{};
	uint32_t vertexCount = 0;
	uint32_t indexCount = 0;

	std::string textureFilePath{};
	uint32_t textureIndex = 0;
};

struct ModelData {
	std::vector<Mesh> meshes;

	// CPU側の階層構造やスキンクラスター用の生データ
	Node rootNode{};
	std::unordered_map<std::string, JointWeightData> skinClusterData{};
};