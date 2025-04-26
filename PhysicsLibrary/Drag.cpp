#include "pch.h"
#include "Drag.h"

Drag::Drag(float k1, float k2)
    : k1(k1), k2(k2) {}

void Drag::updateForce(Particle* particle, float duration) {
    Vector3 velocity = particle->getVelocity();

    float speed = velocity.magnitude();
    if (speed == 0.0f) return; //no drag on a stationary object

    //calculate drag coefficient
    float dragCoeff = k1 * speed + k2 * speed * speed;

    //calculate final drag force direction and magnitude
    Vector3 dragForce = velocity.normalized() * -dragCoeff;

    particle->addForce(dragForce);
}