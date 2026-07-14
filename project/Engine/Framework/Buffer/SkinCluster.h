#pragma once

namespace MyEngine::Rendering {
	// 頂点のウェイトデータ
	struct VertexWeightData {
		float weight;
		uint32_t vertexIndex;
	};

	// ジョイントのウェイトデータ
	struct JointWeightData {
		Matrix4x4 inverseBindPoseMatrix;
		std::vector<VertexWeightData> vertexWeights;
	};

	// jointの影響を受ける最大数(大体4つで問題なし)
	const uint32_t kNumMaxInfluence = 4;

	// VertexInfluence構造体
	struct VertexInfluence {
		std::array<float, kNumMaxInfluence> weights;
		std::array<int32_t, kNumMaxInfluence> jointIndices;
	};

	// WellForGPU構造体
	struct WellForGPU {
		Matrix4x4 skeletonSpaceMatrix;	// 位置用
		Matrix4x4 skeletonSpaceInverseTransposeMatrix;	// 法線用
	};

	// SkinClusterデータ
	struct SkinClusterData {
		std::vector<VertexInfluence> vertexInfluences; // 頂点数分
		std::vector<Matrix4x4> inverseBindMatrices;    // ジョイント数分
		std::vector<std::string> jointNames;          // ジョイント数分
	};

	// SkinCluster構造体
	// ※現在リファクタリング中のため一時的にコメントアウト
	/*
	class DxCommon;
	struct SkinClusterResource {
	public:
		SkinClusterResource() = default;
		~SkinClusterResource() { Release(); }
		void Initialize(DxCommon* dxCommon, const SkinClusterData& cpuData, UINT srvDescriptorRow) {}
		void UpdatePalette(const Skeleton& skeleton, const std::vector<Matrix4x4>& inverseBindMatrices) {}
		void Release() {}
		const D3D12_VERTEX_BUFFER_VIEW& GetInfluenceView() const { static D3D12_VERTEX_BUFFER_VIEW view{}; return view; }
		D3D12_GPU_DESCRIPTOR_HANDLE GetPaletteSRVHandle() const { static D3D12_GPU_DESCRIPTOR_HANDLE handle{}; return handle; }
	};
	*/
}