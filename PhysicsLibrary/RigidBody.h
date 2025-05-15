#pragma once
#include "Vector3.h"
#include "Matrix3.h"
#include "Quaternion.h"

class RigidBody {
private:
    // Linear
    Vector3 position;
    Vector3 velocity;
    Vector3 acceleration;
    Vector3 forceAccum;
    float inverseMass;
    float linearDamping;

    // Angular
    Quaternion orientation;
    Vector3 angularVelocity;
    Vector3 torqueAccum;
    Matrix3 inverseInertiaTensor;
    float angularDamping;

public:
    RigidBody();

    void integrate(float duration);
    void addForce(const Vector3& force);
    void addTorque(const Vector3& torque);
    void clearAccumulators();

    // Setters
    void setPosition(const Vector3& pos);
    void setVelocity(const Vector3& vel);
    void setAcceleration(const Vector3& acc);
    void setOrientation(const Quaternion& q);
    void setAngularVelocity(const Vector3& av);
    void setMass(float mass);
    void setInverseMass(float invMass);
    void setLinearDamping(float d);
    void setAngularDamping(float d);
    void setInverseInertiaTensor(const Matrix3& tensor);

    // Getters
    Vector3 getPosition() const;
    Vector3 getVelocity() const;
    Vector3 getAcceleration() const;
    Vector3 getAngularVelocity() const;
    Quaternion getOrientation() const;
    float getMass() const;
    float getInverseMass() const;
    Matrix3 getInverseInertiaTensor() const;
    bool hasFiniteMass() const;
};
