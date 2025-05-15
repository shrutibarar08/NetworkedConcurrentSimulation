#pragma once
#include "Vector3.h"
<<<<<<< Updated upstream

class Particle
{
private:
	Vector3 position;
	Vector3 velocity;
	Vector3 acceleration;
	Vector3 forceAccum;
	float inverseMass;
	float damping;

=======
class Particle {
>>>>>>> Stashed changes
public:
    Particle();
    void integrate(float duration);
    void addForce(const Vector3& force);
    void clearAccumulator();
    // Setters
    void setPosition(const Vector3& pos);
    void setVelocity(const Vector3& vel);
    void setAcceleration(const Vector3& acc);
    void setMass(float mass);
    void setInverseMass(float invMass);
    void setDamping(float d);
<<<<<<< Updated upstream

=======
>>>>>>> Stashed changes
    // Getters
    Vector3 getPosition() const;
    Vector3 getVelocity() const;
    Vector3 getAcceleration() const;
    float getMass() const;
    float getInverseMass() const;
    float getDamping() const;
    bool hasFiniteMass() const;
<<<<<<< Updated upstream
};
=======
>>>>>>> Stashed changes

private:
    Vector3 position;
    Vector3 velocity;
    Vector3 acceleration;
    Vector3 forceAccum;
    float inverseMass;
    float damping;
};
