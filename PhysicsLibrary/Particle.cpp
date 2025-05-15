#include "pch.h"
#include "Particle.h"
#include <cmath>

Particle::Particle()
    : position(), velocity(), acceleration(), forceAccum(),
    inverseMass(1.0f), damping(0.99f) {
}

void Particle::integrate(float duration) {
    if (inverseMass <= 0.0f) return;

    // Update position
    position += velocity * duration;

    // Acceleration from force
    Vector3 resultingAcc = acceleration + forceAccum * inverseMass;
    velocity += resultingAcc * duration;

    // Apply damping
    velocity *= std::pow(damping, duration);

    // Clear accumulated forces
    clearAccumulator();
}

void Particle::addForce(const Vector3& force) {
    forceAccum += force;
}

void Particle::clearAccumulator() {
    forceAccum.clear();
}

// Setters
void Particle::setPosition(const Vector3& pos) { position = pos; }
void Particle::setVelocity(const Vector3& vel) { velocity = vel; }
void Particle::setAcceleration(const Vector3& acc) { acceleration = acc; }
void Particle::setMass(float mass) { inverseMass = (mass > 0.0f) ? 1.0f / mass : 0.0f; }
void Particle::setInverseMass(float invMass) { inverseMass = invMass; }
void Particle::setDamping(float d) { damping = d; }

// Getters
Vector3 Particle::getPosition() const { return position; }
Vector3 Particle::getVelocity() const { return velocity; }
Vector3 Particle::getAcceleration() const { return acceleration; }
float Particle::getMass() const { return (inverseMass > 0.0f) ? 1.0f / inverseMass : INFINITY; }
float Particle::getInverseMass() const { return inverseMass; }
float Particle::getDamping() const { return damping; }
bool Particle::hasFiniteMass() const { return inverseMass > 0.0f; }
