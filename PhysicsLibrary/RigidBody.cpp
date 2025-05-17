#include "pch.h"
#include "pch.h"
#include "RigidBody.h"
#include <cmath>

RigidBody::RigidBody()
    : position(), velocity(), acceleration(), forceAccum(),
    inverseMass(1.0f), linearDamping(0.98f),
    orientation(1, 0, 0, 0), angularVelocity(), torqueAccum(),
    angularDamping(0.9f) {
    inverseInertiaTensor.setIdentity();
    calculateDerivedData();
}

void RigidBody::integrate(float duration) {
    if (inverseMass <= 0.0f) return;

    // Linear motion
    position += velocity * duration;
    Vector3 linearAcc = acceleration + forceAccum * inverseMass;
    velocity += linearAcc * duration;
    velocity *= std::pow(linearDamping, duration);

    // Angular motion
    Vector3 angularAcc = inverseInertiaTensorWorld.transform(torqueAccum);
    angularVelocity += angularAcc * duration;
    angularVelocity *= std::pow(angularDamping, duration);

    Quaternion deltaRot(0, angularVelocity.x, angularVelocity.y, angularVelocity.z);
    deltaRot = deltaRot * orientation;
    orientation += deltaRot * 0.5f * duration;
    orientation.normalize();

    calculateDerivedData();
    clearAccumulators();
}

void RigidBody::addForce(const Vector3& force) {
    forceAccum += force;
}

void RigidBody::addTorque(const Vector3& torque) {
    torqueAccum += torque;
}

void RigidBody::integrate(float dt, IntegrationType type) {
    if (inverseMass <= 0.0f) return;

    switch (type) {
    case IntegrationType::Euler: {
        position += velocity * dt;
        velocity += (acceleration + forceAccum * inverseMass) * dt;
        break;
    }
    case IntegrationType::SemiImplicitEuler: {
        velocity += (acceleration + forceAccum * inverseMass) * dt;
        position += velocity * dt;
        break;
    }
    case IntegrationType::Verlet: {
        static Vector3 lastPosition = position; // crude; per-body state should be better managed
        Vector3 newPosition = position + (position - lastPosition) + (acceleration + forceAccum * inverseMass) * dt * dt;
        lastPosition = position;
        position = newPosition;
        break;
    }
    }

    velocity *= std::pow(linearDamping, dt);
    angularVelocity += (inverseInertiaTensor * torqueAccum) * dt;
    angularVelocity *= std::pow(angularDamping, dt);
    orientation.addScaledVector(angularVelocity, dt);
    orientation.normalize();

    clearAccumulators();
}

void RigidBody::calculateDerivedData() {
    orientation.normalize();
    transformMatrix = Matrix3::rotationMatrix(orientation);
    inverseInertiaTensorWorld = transformMatrix * inverseInertiaTensor * transformMatrix.transpose();
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
    inverseMass = invMass; 
}
void RigidBody::setLinearDamping(float d) {
    linearDamping = d; 
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
    return inverseMass > 0.0f;}
