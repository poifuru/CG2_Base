#include "IParticleMesh.h"
#include <numbers>
#include <cmath>

static inline const uint32_t kParticleVertexNum = 4;
static inline const uint32_t kParticleIndexNum = 6;

static inline const uint32_t kRingDivide = 32;
static inline const float kOutRadius = 1.0f;
static inline const float kInnerRadius = 0.2f;
static inline const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / float(kRingDivide);

// ----------------------------------------------------
// Quad 形状（四角形ポリゴン）
// ----------------------------------------------------
void QuadParticleMesh::GenerateMesh(std::vector<ParticleVertex>& vertices, std::vector<uint32_t>& indices) {
	vertices.resize(kParticleVertexNum);
	vertices[0] = {	// 左上
		{-1.0f, 1.0f, 0.0f, 1.0f},
		{0.0f, 0.0f},
	};
	vertices[1] = {	// 右上
		{1.0f, 1.0f, 0.0f, 1.0f},
		{1.0f, 0.0f},
	};
	vertices[2] = {	// 左下
		{-1.0f, -1.0f, 0.0f, 1.0f},
		{0.0f, 1.0f},
	};
	vertices[3] = {	// 右下
		{1.0f, -1.0f, 0.0f, 1.0f},
		{1.0f, 1.0f},
	};

	indices = {
		0, 1, 2,
		1, 3, 2
	};
}

// ----------------------------------------------------
// Ring 形状（リング）
// ----------------------------------------------------
void RingParticleMesh::GenerateMesh(std::vector<ParticleVertex>& vertices, std::vector<uint32_t>& indices) {
	uint32_t totalVertices = (kRingDivide + 1) * 2; 
	uint32_t totalIndices = kRingDivide * 6;
	vertices.resize(totalVertices);
	indices.resize(totalIndices);

	for (uint32_t i = 0; i <= kRingDivide; ++i) {
		float angle = float(i) * radianPerDivide;
		float cosAngle = std::cos(angle);
		float sinAngle = std::sin(angle);
		
		// 内周の頂点
		vertices[i].position = { cosAngle * kInnerRadius, sinAngle * kInnerRadius, 0.0f, 1.0f };
		vertices[i].texcoord = { float(i) / float(kRingDivide), 1.0f };
		
		// 外周の頂点
		vertices[i + (kRingDivide + 1)].position = { cosAngle * kOutRadius, sinAngle * kOutRadius, 0.0f, 1.0f };
		vertices[i + (kRingDivide + 1)].texcoord = { float(i) / float(kRingDivide), 0.0f };
	}

	for (uint32_t i = 0; i < kRingDivide; ++i) {
		uint32_t currentInner = i;
		uint32_t nextInner = i + 1;
		uint32_t currentOuter = i + (kRingDivide + 1);
		uint32_t nextOuter = nextInner + (kRingDivide + 1);

		indices[i * 6 + 0] = currentInner;
		indices[i * 6 + 1] = currentOuter;
		indices[i * 6 + 2] = nextInner;
		indices[i * 6 + 3] = currentOuter;
		indices[i * 6 + 4] = nextOuter;
		indices[i * 6 + 5] = nextInner;
	}
}

uint32_t RingParticleMesh::GetIndexCount() const {
	return kRingDivide * 6;
}

// ----------------------------------------------------
// Cylinder 形状（シリンダーチューブ）
// ----------------------------------------------------
void CylinderParticleMesh::GenerateMesh(std::vector<ParticleVertex>& vertices, std::vector<uint32_t>& indices) {
	uint32_t totalVertices = (kRingDivide + 1) * 2; 
	uint32_t totalIndices = kRingDivide * 6;
	vertices.resize(totalVertices);
	indices.resize(totalIndices);

	for (uint32_t i = 0; i <= kRingDivide; ++i) {
		float angle = float(i) * radianPerDivide;
		float cosAngle = std::cos(angle);
		float sinAngle = std::sin(angle);

		// 底面（Z = -1.0f）の周上の点
		vertices[i].position = { cosAngle * kOutRadius, sinAngle * kOutRadius, -1.0f, 1.0f };
		vertices[i].texcoord = { float(i) / float(kRingDivide), 1.0f };

		// 上面（Z = 1.0f）の周上の点
		vertices[i + (kRingDivide + 1)].position = { cosAngle * kOutRadius, sinAngle * kOutRadius, 1.0f, 1.0f };
		vertices[i + (kRingDivide + 1)].texcoord = { float(i) / float(kRingDivide), 0.0f };
	}

	for (uint32_t i = 0; i < kRingDivide; ++i) {
		uint32_t currentInner = i;
		uint32_t nextInner = i + 1;
		uint32_t currentOuter = i + (kRingDivide + 1);
		uint32_t nextOuter = nextInner + (kRingDivide + 1);

		indices[i * 6 + 0] = currentInner;
		indices[i * 6 + 1] = currentOuter;
		indices[i * 6 + 2] = nextInner;
		indices[i * 6 + 3] = currentOuter;
		indices[i * 6 + 4] = nextOuter;
		indices[i * 6 + 5] = nextInner;
	}
}

uint32_t CylinderParticleMesh::GetIndexCount() const {
	return kRingDivide * 6;
}
