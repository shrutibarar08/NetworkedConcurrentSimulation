#include "pch.h"
#include "Gravity.h"

Gravity::Gravity(const Vector3& g) : gravity(g) {}

void Gravity::updateForce(Particle* particle, float duration) {
    if (!particle->hasFiniteMass()) return;
    particle->addForce(gravity * particle->getMass());
}
