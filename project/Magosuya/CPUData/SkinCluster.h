#pragma once
#include <vector>
#include <array>
#include <span>
#include "Buffer.h"

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

// SkinCluster構造体
struct SkinCluster {
	std::vector<Matrix4x4> inverseBinePoseMatrices;

	ComPtr<ID3D12Resource> influenceResource;
	D3D12_VERTEX_BUFFER_VIEW influenceBufferView;
	std::span<VertexInfluence> mappedInfluence;

	ComPtr<ID3D12Resource> paletteResource;
	std::span<WellForGPU> mappedPalette;
	std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> paletteSrvHandle;
};