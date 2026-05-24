#pragma once
#include <vector>
#include <list>
#include "DxCommon.h"
#include "Buffer.h"
#include "ParticleData.h"
#include "MaterialData.h"

class IParticleField;

class ParticleGroup {
public:
	ParticleGroup(DxCommon* dxCommon);
	~ParticleGroup();

	void Initialize();
	void Update(float deltaTime);
	void Draw();

	// パーティクルを追加する（エミッターから呼ばれる）
	void AddParticle(const ParticleData& particle);

	// このグループに影響を与えるフィールドを登録する
	void AddField(IParticleField* field);

private:
	DxCommon* dxCommon_ = nullptr;

	// 自作のStructuredBufferを活用！
	StructuredBuffer<ParticleForGPU> instancingBuffer_;
	MaterialResource materialBuffer_;

	std::list<ParticleData> particles_;
	std::vector<IParticleField*> fields_; // 適用するフィールドのポインタ配列

	// 出せるパーティクルの最大数
	const uint32_t kMaxParticleNum_ = 5000;
};