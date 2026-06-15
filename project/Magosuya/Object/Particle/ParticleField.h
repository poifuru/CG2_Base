#pragma once
#include "IParticleField.h"
#include <string>

class GravityField : public IParticleField {
public:
	GravityField(const Vector3& gravity) : gravity_(gravity) {}

	void Apply(ParticleData& particle) override;

	void ImGui() override;

private:
	Vector3 gravity_;
};