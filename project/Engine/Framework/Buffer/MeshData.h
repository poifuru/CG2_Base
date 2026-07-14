#pragma once
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Skeleton.h"
#include "SkinCluster.h"
#include "RenderingModel.h"

// 頂点データの構造体
struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

namespace MyEngine::Rendering {
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

	// 形状データだけをもつMesh構造体
	struct Mesh {
		MeshData meshData;
		MeshResource meshResource;

		D3D12_VERTEX_BUFFER_VIEW vbView{};
		D3D12_INDEX_BUFFER_VIEW  ibView{};
		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;

		MyEngine::Rendering::InputLayoutType inputLayout = MyEngine::Rendering::InputLayoutType::Standard3D;
	};

	struct ModelData {
		std::vector<Mesh> meshes;						// 各メッシュの形状データのリスト
		std::vector<std::string> defaultTexturePaths;	// 各メッシュの初期テクスチャのリスト
		std::vector<uint32_t> defaultTextureIndices;	// 各メッシュの初期テクスチャインデックスのリスト

		// CPU側の階層構造やスキンクラスター用の生データ
		Node rootNode{};
		std::unordered_map<std::string, JointWeightData> skinClusterData{};
	};
}