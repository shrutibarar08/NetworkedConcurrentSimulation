#pragma once
#include "ForceGenerator.h"
#include "Vector3.h"
#include "Particle.h"
class Gravity : public ForceGenerator
{
private:
	Vector3 gravity;

public:

	Gravity(const Vector3& gravity);

	void updateForce(Particle* particle, float duration) override;
};
