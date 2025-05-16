#pragma once
#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix3.h"
#include "IntegrationType.h"

class RigidBody {
public:
    RigidBody();

    void integrate(float duration);
    void calculateDerivedData();

    void clearAccumulators();
    void addForce(const Vector3& force);
    void addTorque(const Vector3& torque);
    void integrate(float dt, IntegrationType type = IntegrationType::SemiImplicitEuler);


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

private:
    Vector3 position;
    Vector3 velocity;
    Vector3 acceleration;
    Vector3 forceAccum;

    float inverseMass;
    float linearDamping;

    Quaternion orientation;
    Vector3 angularVelocity;
    Vector3 torqueAccum;
    float angularDamping;

    Matrix3 inverseInertiaTensor;
    Matrix3 inverseInertiaTensorWorld;
    Matrix3 transformMatrix;

};
