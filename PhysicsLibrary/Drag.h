#pragma once
#include "ForceGenerator.h"

class Drag : public ForceGenerator
{
public:
	Drag(float k1, float k2);
	void UpdateForce(ICollider* collider, float duration) override;

private:
	float K1; 
	float K2; 
};

