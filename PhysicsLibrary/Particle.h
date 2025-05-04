#pragma once
#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix3.h"

class Particle
{
private:
	Vector3 position;
	Vector3 velocity;
	Vector3 acceleration;
	Vector3 forceAccum;
	float inverseMass;
	float damping;

    // Rotation-related
    Quaternion orientation;
    Vector3 angularVelocity;
    Vector3 torqueAccum;
    Matrix3 inverseInertiaTensor;

public:

	Particle();

    void integrate(float duration);

    void addForce(const Vector3& force);
    void addTorque(const Vector3& torque);
    void clearAccumulator();


    // Setters
    void setPosition(const Vector3& pos);
    void setVelocity(const Vector3& vel);
    void setAcceleration(const Vector3& acc);
    void setMass(float mass);
    void setInverseMass(float invMass);
    void setDamping(float d);
    void setOrientation(const Quaternion& q);
    void setAngularVelocity(const Vector3& av);
    void setInverseInertiaTensor(const Matrix3& tensor);

    // Getters
    Vector3 getPosition() const;
    Vector3 getVelocity() const;
    Vector3 getAcceleration() const;
    float getMass() const;
    float getInverseMass() const;
    float getDamping() const;
    bool hasFiniteMass() const;
    Quaternion getOrientation() const;
    Vector3 getAngularVelocity() const;
    Matrix3 getInverseInertiaTensor() const;
};

