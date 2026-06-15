#include "ParticleField.h"

void GravityField::Apply(ParticleData& particle) {
	particle.acceleration += gravity_;
}

void GravityField::ImGui() {
#ifdef USEIMGUI

#endif
}