#include "pch.h"
#include "Gravity.h"

Gravity::Gravity(const Vector3& gravity) : gravity(gravity) {}

void Gravity::updateForce(Particle* particle, float duration)
{
	if (!particle->hasFiniteMass()) return;

	//f = m*g 
	Vector3 force = gravity * particle->getMass();
	particle->addForce(force);
}
