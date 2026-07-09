#pragma once
#include <vector>
#include <array>
#include "Buffer.h"
#include "DescriptorHandle.h"
#include "SRVManager.h"
#include "MathFunction.h"
#include "Skeleton.h"
#include "struct.h"

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
		SRVManager::GetInstance()->CreateSRVStructuredBuffer(srvIndex_, paletteBuffer_.Get(), jointCount_, sizeof(WellForGPU));
		paletteSrvHandle_.gpu = SRVManager::GetInstance()->GetGPUDescriptorHandle(srvIndex_);	// 作ったSRVのハンドルを保持しておく

		// InfluenceBufferの初期化とデータ転送
		influenceBuffer_.Initialize(dxCommon, cpuData.vertexInfluences);
		influenceBuffer_.Update(cpuData.vertexInfluences);

		// スキニング後の頂点用リソースの初期化 (UAV/VBV兼用)
		UINT vertexCount = static_cast<UINT>(cpuData.vertexInfluences.size());
		D3D12_RESOURCE_DESC resDesc = {};
		resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resDesc.Width = sizeof(VertexData) * vertexCount;
		resDesc.Height = 1;
		resDesc.DepthOrArraySize = 1;
		resDesc.MipLevels = 1;
		resDesc.Format = DXGI_FORMAT_UNKNOWN;
		resDesc.SampleDesc.Count = 1;
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

		dxCommon->GetDevice()->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
			nullptr,
			IID_PPV_ARGS(&skinnedVertexBuffer_)
		);

		// VBVの設定
		skinnedVBView_.BufferLocation = skinnedVertexBuffer_->GetGPUVirtualAddress();
		skinnedVBView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * vertexCount);
		skinnedVBView_.StrideInBytes = sizeof(VertexData);
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

		if (srvIndex_ != 0xFFFFFFFF) {
			SRVManager::GetInstance()->Free(srvIndex_);
			srvIndex_ = 0xFFFFFFFF;
		}

		skinnedVertexBuffer_ = nullptr;
		skinnedVBView_ = {};
	}

	// ゲッター
	const D3D12_VERTEX_BUFFER_VIEW& GetInfluenceView() const { return influenceBuffer_.GetView(); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetPaletteSRVHandle() const { return paletteSrvHandle_.gpu; }
	const D3D12_VERTEX_BUFFER_VIEW& GetSkinnedVBView() const { return skinnedVBView_; }
	ID3D12Resource* GetSkinnedVertexBuffer() const { return skinnedVertexBuffer_.Get(); }
	ID3D12Resource* GetInfluenceBuffer() const { return influenceBuffer_.GetResource(); }
	ID3D12Resource* GetPaletteBuffer() const { return paletteBuffer_.Get(); }

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
			skinnedVertexBuffer_ = std::move(other.skinnedVertexBuffer_);
			skinnedVBView_ = other.skinnedVBView_;
			paletteMappedData_ = other.paletteMappedData_;
			paletteSrvHandle_.gpu = other.paletteSrvHandle_.gpu;
			jointCount_ = other.jointCount_;

			other.paletteMappedData_ = nullptr;
			other.paletteSrvHandle_.gpu = {};
			other.jointCount_ = 0;
			other.skinnedVBView_ = {};
		}
		return *this;
	}

private:
	// パレット用
	ComPtr<ID3D12Resource> paletteBuffer_;
	void* paletteMappedData_ = nullptr;
	uint32_t srvIndex_ = 0xFFFFFFFF; // 0xFFFFFFFF で「まだ何も割り当てられていない」ことを表す
	DescriptorHandle paletteSrvHandle_{};
	UINT jointCount_ = 0;

	// 頂点影響度(別ストリーム兆点バッファ)用
	VertexBuffer<VertexInfluence> influenceBuffer_;

	// CSスキニング用
	ComPtr<ID3D12Resource> skinnedVertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW skinnedVBView_{};
};