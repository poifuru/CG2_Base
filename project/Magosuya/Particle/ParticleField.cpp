#include "ParticleField.h"

void GravityField::Apply(ParticleData& particle) {
	particle.acceleration += gravity_;
}