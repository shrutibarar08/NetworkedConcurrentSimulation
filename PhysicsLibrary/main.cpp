#include "pch.h"
#include <iostream>
#include "Vector3.h"
#include "Particle.h"
#include "Gravity.h"

int main() {
    Particle particle;
    particle.setMass(2.0f); // kg
    particle.setPosition(Vector3(0.0f, 10.0f, 0.0f));
    particle.setVelocity(Vector3(0.0f, 0.0f, 0.0f));
    particle.setAcceleration(Vector3(0.0f, 0.0f, 0.0f));

    Gravity gravity(Vector3(0.0f, -9.81f, 0.0f));

    //Simulate single frame (1 second duration)
    float duration = 1.0f;

    //Apply gravity
    gravity.updateForce(&particle, duration);

    particle.integrate(duration);

    Vector3 pos = particle.getPosition();
    Vector3 vel = particle.getVelocity();

    std::cout << "Position: (" << pos.getX() << ", " << pos.getY() << ", " << pos.getZ() << ")\n";
    std::cout << "Velocity: (" << vel.getX() << ", " << vel.getY() << ", " << vel.getZ() << ")\n";


    return 0;
}