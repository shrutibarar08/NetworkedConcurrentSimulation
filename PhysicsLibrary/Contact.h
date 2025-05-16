#pragma once  
#include "Vector3.h"  
#include "RigidBody.h"  

class Contact {  
public:  
   RigidBody* body[2] = { nullptr, nullptr };  
   Vector3 contactPoint;  
   Vector3 contactNormal;  
   float penetration = 0.0f;  
   float restitution = 0.0f;  
   float friction = 0.0f;  

   void resolve(float duration);  
   float calculateSeparatingVelocity() const; 

private:  
   void resolveVelocity(float duration);  
   void resolveInterpenetration(float duration);  
};