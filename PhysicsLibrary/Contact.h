#pragma once  
#include "DirectXMath.h"  
#include "RigidBody.h"  


class Contact
{
public:
   void Resolve(float duration);  
   float CalculateSeparatingVelocity() const; 

private:  
   void ResolveVelocity(float duration);  
   void ResolveInterpenetration(float duration) const;

public:
	RigidBody* Body[2] = { nullptr, nullptr };
	DirectX::XMFLOAT3 ContactPoint;
	DirectX::XMFLOAT3 ContactNormal;
	float Penetration = 0.0f;
	float Restitution = 0.0f;
	float Friction = 0.0f;
};
