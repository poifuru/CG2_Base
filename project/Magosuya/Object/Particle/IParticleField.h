#pragma once
#include <string>
#include "struct.h"
#include "ParticleData.h"

class IParticleField {
public:
	virtual ~IParticleField() = default;

	// パーティクル一つに対して影響を与える純粋仮想関数
	virtual void Apply(ParticleData& data) = 0;

	virtual void ImGui() = 0;

	std::string GetName() const { return name_; }

protected:
	std::string name_{};
};