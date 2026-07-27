#pragma once
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Skeleton.h"
#include "SkinCluster.h"
#include "RenderingModel.h"
#include "Geometry.h"

// 頂点データの構造体
struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

namespace MyEngine::Rendering {
	// 描画パラメータだけを持つMesh基底クラス
	struct Mesh {
		virtual ~Mesh() = default;

		D3D12_VERTEX_BUFFER_VIEW vbView{};
		D3D12_INDEX_BUFFER_VIEW  ibView{};
		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;
		MyEngine::Rendering::InputLayoutType inputLayout = MyEngine::Rendering::InputLayoutType::Standard3D;

		AABB localBounds;
	};

	// 任意の頂点型を受け取るテンプレートメッシュ
	template <typename T>
	struct GenericMesh : public Mesh {
		// デバイスを受け取ってバッファを生成・データ転送する初期化関数
		void Initialize(ID3D12Device* device, const std::vector<T>& verts, const std::vector<uint32_t>& inds) {
			vertices = verts;
			indices = inds;

			vertexBuffer.Initialize(device, vertices.size());
			vertexBuffer.Update(vertices);

			vbView = vertexBuffer.GetView();
			vertexCount = static_cast<uint32_t>(vertices.size());

			if(!indices.empty()) {
				indexBuffer.Initialize(device, indices.size());
				indexBuffer.Update(indices);

				ibView = indexBuffer.GetView();
				indexCount = static_cast<uint32_t>(indices.size());
			}

			// AABB の自動計算
			if (!vertices.empty()) {
				localBounds.min.x = vertices[0].position.x;
				localBounds.min.y = vertices[0].position.y;
				localBounds.min.z = vertices[0].position.z;
				localBounds.max.x = vertices[0].position.x;
				localBounds.max.y = vertices[0].position.y;
				localBounds.max.z = vertices[0].position.z;

				for (const auto& v : vertices) {
					localBounds.min.x = (std::min)(localBounds.min.x, v.position.x);
					localBounds.min.y = (std::min)(localBounds.min.y, v.position.y);
					localBounds.min.z = (std::min)(localBounds.min.z, v.position.z);

					localBounds.max.x = (std::max)(localBounds.max.x, v.position.x);
					localBounds.max.y = (std::max)(localBounds.max.y, v.position.y);
					localBounds.max.z = (std::max)(localBounds.max.z, v.position.z);
				}
			}
		}

		// 動的更新用の関数 (毎フレーム書き換える用)
		void UpdateVertices(const std::vector<T>& verts) {
			vertices = verts;
			vertexBuffer.Update(vertices);
			vertexCount = static_cast<uint32_t>(vertices.size());
			vbView = vertexBuffer.GetView();
		}

		// 変数
		std::vector<T> vertices;
		std::vector<uint32_t> indices;
		VertexBuffer<T> vertexBuffer;
		IndexBuffer<uint32_t> indexBuffer;
	};

	// 3Dモデル用のエイリアス
	using StaticMesh = GenericMesh<VertexData>;

	struct ModelData {
		// 全メッシュの AABB を合体させて 1 つの大きな AABB を返す関数
		AABB GetTotalAABB() const {
			AABB total;
			if (meshes.empty()) return total;
			total = meshes[0].localBounds; // 1個目で初期化
			for (size_t i = 1; i < meshes.size(); ++i) {
				total.min.x = (std::min)(total.min.x, meshes[i].localBounds.min.x);
				total.min.y = (std::min)(total.min.y, meshes[i].localBounds.min.y);
				total.min.z = (std::min)(total.min.z, meshes[i].localBounds.min.z);

				total.max.x = (std::max)(total.max.x, meshes[i].localBounds.max.x);
				total.max.y = (std::max)(total.max.y, meshes[i].localBounds.max.y);
				total.max.z = (std::max)(total.max.z, meshes[i].localBounds.max.z);
			}
			return total;
		}

		std::vector<StaticMesh> meshes;					// 各メッシュの形状データのリスト
		std::vector<std::string> defaultTexturePaths;	// 各メッシュの初期テクスチャのリスト
		std::vector<uint32_t> defaultTextureIndices;	// 各メッシュの初期テクスチャインデックスのリスト

		// CPU側の階層構造やスキンクラスター用の生データ
		Node rootNode{};
		std::unordered_map<std::string, JointWeightData> skinClusterData{};
	};
}