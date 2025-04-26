#include "pch.h"
#include "Contact.h"

void Contact::resolve(float duration)
{
	resolveVelocity(duration);
	resolveInterpenetration(duration);
}

float Contact::calculateSeparatingVelocity() const
{
    Vector3 relativeVelocity = particle[0]->getVelocity();
    if (particle[1]) {
        relativeVelocity -= particle[1]->getVelocity();
    }
    return relativeVelocity.dot(contactNormal);
}

void Contact::resolveVelocity(float duration)
{
    float separatingVelocity = calculateSeparatingVelocity();

    if (separatingVelocity > 0) {
        return;
    }

    float newSepVelocity = -separatingVelocity * restitution;

    // Some extra velocity due to acceleration buildup
    Vector3 accCausedVelocity = particle[0]->getAcceleration();
    if (particle[1]) {
        accCausedVelocity -= particle[1]->getAcceleration();
    }
    float accSepVelocity = accCausedVelocity.dot(contactNormal) * duration;

    // If closing velocity due to acceleration is negative, remove it from bounce
    if (accSepVelocity < 0) {
        newSepVelocity += restitution * accSepVelocity;
        if (newSepVelocity < 0) newSepVelocity = 0;
    }

    float deltaVelocity = newSepVelocity - separatingVelocity;

    // Calculate impulse needed
    float totalInverseMass = particle[0]->getInverseMass();
    if (particle[1]) {
        totalInverseMass += particle[1]->getInverseMass();
    }

    // Infinite mass check
    if (totalInverseMass <= 0) return;

    float impulse = deltaVelocity / totalInverseMass;
    Vector3 impulsePerIMass = contactNormal * impulse;

    // Apply impulse
    particle[0]->setVelocity(particle[0]->getVelocity() + impulsePerIMass * particle[0]->getInverseMass());
    if (particle[1]) {
        particle[1]->setVelocity(particle[1]->getVelocity() - impulsePerIMass * particle[1]->getInverseMass());
    }
}

void Contact::resolveInterpenetration(float duration)
{
    if (penetration <= 0) return;

    float totalInverseMass = particle[0]->getInverseMass();
    if (particle[1]) {
        totalInverseMass += particle[1]->getInverseMass();
    }

    if (totalInverseMass <= 0) return; // infinite mass, no movement

    Vector3 movePerIMass = contactNormal * (penetration / totalInverseMass);

    particleMovement[0] = movePerIMass * particle[0]->getInverseMass();
    if (particle[1]) {
        particleMovement[1] = movePerIMass * -particle[1]->getInverseMass();
    }
    else {
        particleMovement[1].clear();
    }

    // Apply the movement
    particle[0]->setPosition(particle[0]->getPosition() + particleMovement[0]);
    if (particle[1]) {
        particle[1]->setPosition(particle[1]->getPosition() + particleMovement[1]);
    }

}
