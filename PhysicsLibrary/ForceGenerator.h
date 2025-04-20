#pragma once
#include "Particle.h"

class ForceGenerator
{
public:
	virtual ~ForceGenerator() = default;

	virtual void updateForce(class Particle* particle, float duration) = 0;

};

