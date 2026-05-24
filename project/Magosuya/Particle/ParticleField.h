#pragma once
#include "IParticleField.h"
class GravityField : public IParticleField {
public:
	GravityField(const Vector3& gravity) : gravity_(gravity) {}

	void Apply(ParticleData& particle) override;

private:
	Vector3 gravity_;
};