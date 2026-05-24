#pragma once
#include "struct.h"
#include "ParticleData.h"

class IParticleField {
public:
	virtual ~IParticleField() = default;

	// パーティクル一つに対して影響を与える純粋仮想関数
	virtual void Apply(ParticleData& data) = 0;
};