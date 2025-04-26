#pragma once
#include "ForceGenerator.h"
#include "Particle.h"
#include "Vector3.h"

class Drag : public ForceGenerator
{
public:
	Drag(float k1, float k2);

	void updateForce(Particle* particle, float duration) override;

private:
	float k1; 
	float k2; 
};

