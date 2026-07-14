#pragma once

namespace MyEngine::Rendering {
	// Node構造体
	struct Node {
		QuaternionTransform transform;
		Matrix4x4 localMatrix;
		std::string name;
		std::vector<Node> children;
	};

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