#pragma once
#include "Vector3.h"
#include "Particle.h"

class Contact
{
public:
	Particle* particle[2];
	float restitution;
	Vector3 contactNormal;
	float penetration;
	Vector3 particleMovement[2];

	void resolve(float duration);

	float calculateSeparatingVelocity() const;

private:
	void resolveVelocity(float duration);
	void resolveInterpenetration(float duration);

};

