#pragma once
#include <vector>
#include <array>
#include "Buffer.h"
#include "DescriptorHandle.h"
#include "SRVManager.h"
#include "MathFunction.h"
#include "Skeleton.h"

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
struct SkinClusterResource {
public:
	SkinClusterResource() = default;

	~SkinClusterResource() { Release(); }

	void Initialize(DxCommon* dxCommon, const SkinClusterData& cpuData, UINT srvDescriptorRow) {
		Release();
		jointCount_ = static_cast<UINT>(cpuData.inverseBindMatrices.size());

		// パレット用構造化バッファの生成(ジョイント数分)
		size_t paletteSize = sizeof(WellForGPU) * jointCount_;
		paletteBuffer_ = dxCommon->CreateBufferResource(paletteSize);
		if(paletteBuffer_) {
			paletteBuffer_->Map(0, nullptr, &paletteMappedData_);
		}

		// SRV作成
		srvIndex_ = SRVManager::GetInstance()->Allocate();
		SRVManager::GetInstance()->CreateSRVStructuredBuffer(srvIndex_, paletteBuffer_.Get(), UINT(paletteSize), sizeof(WellForGPU));
		paletteSrvHandle_.gpu = SRVManager::GetInstance()->GetGPUDescriptorHandle(srvIndex_);	// 作ったSRVのハンドルを保持しておく

		// InfluenceBufferの初期化
		influenceBuffer_.Initialize(dxCommon, cpuData.vertexInfluences);
	}

	void UpdatePalette(const Skeleton& skeleton, const std::vector<Matrix4x4>& inverseBindMatrices) {	
		if(!paletteMappedData_) return;

		// ジョイント数が不一致を起こしていないかチェック
		assert(jointCount_ == skeleton.joints.size());
		assert(jointCount_ == inverseBindMatrices.size());

		WellForGPU* srvData = static_cast<WellForGPU*>(paletteMappedData_);

		for(UINT i = 0; i < jointCount_; ++i) {
			// 位置変形用の行列計算 (初期姿勢の逆行列 × 現在のスケルトン空間の行列)
			srvData[i].skeletonSpaceMatrix = Math::Multiply(inverseBindMatrices[i], skeleton.joints[i].skeletonSpaceMatrix);

			// 法線変形用の行列計算 (位置用行列の逆転置行列)
			srvData[i].skeletonSpaceInverseTransposeMatrix = Math::Transpose(Math::Inverse(srvData[i].skeletonSpaceMatrix));
		}
	}

	void Release() {
		if(paletteBuffer_ && paletteMappedData_) {
			paletteBuffer_->Unmap(0, nullptr);
		}
		paletteBuffer_ = nullptr;
		paletteMappedData_ = nullptr;
		jointCount_ = 0;

		// SRVの解放
		SRVManager::GetInstance()->Free(srvIndex_);
	}

	// ゲッター
	const D3D12_VERTEX_BUFFER_VIEW& GetInfluenceView() const { return influenceBuffer_.GetView(); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetPaletteSRVHandle() const { return paletteSrvHandle_.gpu; }

public:
	// コピー禁止
	SkinClusterResource(const SkinClusterResource&) = delete;
	SkinClusterResource& operator=(const SkinClusterResource&) = delete;

	// ムーブ許可
	SkinClusterResource(SkinClusterResource&& other) noexcept {
		*this = std::move(other);
	}
	SkinClusterResource& operator=(SkinClusterResource&& other) noexcept {
		if(this != &other) {
			Release();
			paletteBuffer_ = std::move(other.paletteBuffer_);
			influenceBuffer_ = std::move(other.influenceBuffer_);
			paletteMappedData_ = other.paletteMappedData_;
			paletteSrvHandle_.gpu = other.paletteSrvHandle_.gpu;
			jointCount_ = other.jointCount_;

			other.paletteMappedData_ = nullptr;
			other.paletteSrvHandle_.gpu = {};
			other.jointCount_ = 0;
		}
		return *this;
	}

private:
	// パレット用
	ComPtr<ID3D12Resource> paletteBuffer_;
	void* paletteMappedData_ = nullptr;
	uint32_t srvIndex_ = 0;
	DescriptorHandle paletteSrvHandle_{};
	UINT jointCount_ = 0;

	// 頂点影響度(別ストリーム兆点バッファ)用
	VertexBuffer<VertexInfluence> influenceBuffer_;
};