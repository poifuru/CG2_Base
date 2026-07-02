#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include "ParticleData.h"

class IParticleMesh {
public:
	virtual ~IParticleMesh() = default;

	// メッシュデータを生成する
	virtual void GenerateMesh(std::vector<ParticleVertex>& vertices, std::vector<uint32_t>& indices) = 0;

	// 描画インデックス数を取得する
	virtual uint32_t GetIndexCount() const = 0;

	// 形状の名前を取得する
	virtual std::string GetName() const = 0;
};

// ----------------------------------------------------
// Quad 形状（四角形ポリゴン）
// ----------------------------------------------------
class QuadParticleMesh : public IParticleMesh {
public:
	void GenerateMesh(std::vector<ParticleVertex>& vertices, std::vector<uint32_t>& indices) override;
	uint32_t GetIndexCount() const override { return 6; }
	std::string GetName() const override { return "Quad"; }
};

// ----------------------------------------------------
// Ring 形状（リング）
// ----------------------------------------------------
class RingParticleMesh : public IParticleMesh {
public:
	void GenerateMesh(std::vector<ParticleVertex>& vertices, std::vector<uint32_t>& indices) override;
	uint32_t GetIndexCount() const override;
	std::string GetName() const override { return "Ring"; }
};

// ----------------------------------------------------
// Cylinder 形状（シリンダーチューブ）
// ----------------------------------------------------
class CylinderParticleMesh : public IParticleMesh {
public:
	void GenerateMesh(std::vector<ParticleVertex>& vertices, std::vector<uint32_t>& indices) override;
	uint32_t GetIndexCount() const override;
	std::string GetName() const override { return "Cylinder"; }
};
