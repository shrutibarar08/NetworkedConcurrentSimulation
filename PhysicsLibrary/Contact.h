#pragma once
#include "Vector3.h"
#include "RigidBody.h"

class Contact {
    
public:
    RigidBody* body[2];

    float restitution;
    float friction;
    float penetration;
    Vector3 contactPoint;
    Vector3 contactNormal;

    Contact();

    float calculateSeparatingVelocity() const;
    void resolve(float duration);
};