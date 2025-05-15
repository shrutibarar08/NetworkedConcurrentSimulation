#include "pch.h"
#include "RigidBody.h"
#include <cmath>

RigidBody::RigidBody()
    : position(), velocity(), acceleration(), forceAccum(),
    inverseMass(1.0f), linearDamping(0.98f),
    orientation(1, 0, 0, 0), angularVelocity(), torqueAccum(),
    angularDamping(0.9f) {
    inverseInertiaTensor.setIdentity();
}

void RigidBody::integrate(float duration) {
    if (inverseMass <= 0.0f) return;

    // Linear
    position += velocity * duration;
    Vector3 linearAcc = acceleration + forceAccum * inverseMass;
    velocity += linearAcc * duration;
    velocity *= std::pow(linearDamping, duration);

    // Angular
    orientation.addScaledVector(angularVelocity, duration);
    orientation.normalize();

    Vector3 angularAcc = inverseInertiaTensor * torqueAccum;
    angularVelocity += angularAcc * duration;
    angularVelocity *= std::pow(angularDamping, duration);

    clearAccumulators();
}

void RigidBody::addForce(const Vector3& force) {
    forceAccum += force;
}

void RigidBody::addTorque(const Vector3& torque) {
    torqueAccum += torque;
}

void RigidBody::clearAccumulators() {
    forceAccum.clear();
    torqueAccum.clear();
}

// Setters
void RigidBody::setPosition(const Vector3& pos) { 
    position = pos; 
}
void RigidBody::setVelocity(const Vector3& vel) { 
    velocity = vel; 
}
void RigidBody::setAcceleration(const Vector3& acc) { 
    acceleration = acc; 
}
void RigidBody::setOrientation(const Quaternion& q) {
    orientation = q; orientation.normalize(); 
}
void RigidBody::setAngularVelocity(const Vector3& av) { 
    angularVelocity = av; 
}
void RigidBody::setMass(float mass) { 
    inverseMass = (mass > 0.0f) ? 1.0f / mass : 0.0f; 
}
void RigidBody::setInverseMass(float invMass) { 
    inverseMass = invMass; }
void RigidBody::setLinearDamping(float d) { linearDamping = d; 
}
void RigidBody::setAngularDamping(float d) { 
    angularDamping = d; 
}
void RigidBody::setInverseInertiaTensor(const Matrix3& tensor) { 
    inverseInertiaTensor = tensor; 
}

// Getters
Vector3 RigidBody::getPosition() const { 
    return position; 
}
Vector3 RigidBody::getVelocity() const { 
    return velocity; 
}
Vector3 RigidBody::getAcceleration() const { 
    return acceleration; 
}
Vector3 RigidBody::getAngularVelocity() const { 
    return angularVelocity; 
}
Quaternion RigidBody::getOrientation() const { 
    return orientation; 
}
float RigidBody::getMass() const { 
    return (inverseMass > 0.0f) ? 1.0f / inverseMass : INFINITY;
}
float RigidBody::getInverseMass() const { 
    return inverseMass; 
}
Matrix3 RigidBody::getInverseInertiaTensor() const { 
    return inverseInertiaTensor; 
}
bool RigidBody::hasFiniteMass() const { 
    return inverseMass > 0.0f; 
}

