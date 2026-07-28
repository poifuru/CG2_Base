#pragma once
#include "MathFunction.h"

namespace MyEngine::Rendering {
	// Node構造体
	struct Node {
		QuaternionTransform transform;
		Matrix4x4 localMatrix;
		std::string name;
		std::vector<Node> children;
		std::vector<uint32_t> meshIndices;
	};

	// ノード名から特定のNodeを検索する関数 (再帰処理)
	inline Node* FindNode(Node* node, const std::string& name) {
		if (!node) return nullptr;
		if (node->name == name) {
			return node;
		}
		for (auto& child : node->children) {
			Node* result = FindNode(&child, name);
			if (result) {
				return result;
			}
		}
		return nullptr;
	}

	// Const版 FindNode
	inline const Node* FindNode(const Node* node, const std::string& name) {
		if (!node) return nullptr;
		if (node->name == name) {
			return node;
		}
		for (const auto& child : node->children) {
			const Node* result = FindNode(&child, name);
			if (result) {
				return result;
			}
		}
		return nullptr;
	}

	// ノードのtransformからlocalMatrixと階層行列を再計算して伝播させる関数
	inline void UpdateNodeTransforms(Node* node, const Matrix4x4& parentMatrix = Math::MakeIdentity4x4()) {
		if (!node) return;

		Matrix4x4 localMat = Math::MakeAffineMatrix(
			node->transform.scale, node->transform.rotate, node->transform.translate
		);
		node->localMatrix = parentMatrix * localMat;

		for (auto& child : node->children) {
			UpdateNodeTransforms(&child, node->localMatrix);
		}
	}

	// Joint構造体
	struct Joint {
		QuaternionTransform transform;	//Transform情報
		Matrix4x4 localMatrix;	// localMatrix
		Matrix4x4 skeletonSpaceMatrix;	 // skeletonSpaceでの変換行列
		std::string name;	// 名前
		std::vector<int32_t> children;	// 子JointのIndexのリスト。いなければ空
		uint32_t index;	// 自身のIndex
		std::optional<int32_t> parent;	// 親JointのIndex。いなければnull
	};

	// Skeleton構造体
	struct Skeleton {
		int32_t	root;	// RootJointのIndex
		std::map<std::string, int32_t> jointMap;	// joint名とIndexとの辞書
		std::vector<Joint> joints;	// 所属しているジョイント
	};
}