#include "pch.h"
#include "Contact.h"

void Contact::resolve(float duration) {
    resolveVelocity(duration);
    resolveInterpenetration(duration);
}

float Contact::calculateSeparatingVelocity() const {
    Vector3 relativeVel = body[0]->getVelocity();
    if (body[1]) relativeVel -= body[1]->getVelocity();
    return relativeVel.dot(contactNormal);
}

void Contact::resolveVelocity(float duration) {
    float separatingVel = calculateSeparatingVelocity();
    if (separatingVel > 0) return; // Already separating

    float newSepVel = -separatingVel * restitution;
    float deltaVel = newSepVel - separatingVel;

    float totalInverseMass = body[0]->getInverseMass();
    if (body[1]) totalInverseMass += body[1]->getInverseMass();
    if (totalInverseMass <= 0) return;

    float impulse = deltaVel / totalInverseMass;
    Vector3 impulsePerIMass = contactNormal * impulse;

    body[0]->setVelocity(body[0]->getVelocity() + impulsePerIMass * body[0]->getInverseMass());
    if (body[1]) {
        body[1]->setVelocity(body[1]->getVelocity() - impulsePerIMass * body[1]->getInverseMass());
    }
}

void Contact::resolveInterpenetration(float duration) {
    float separatingVel = calculateSeparatingVelocity();
    if (separatingVel > 0) return; 

    // Calculate new separating velocity
    float newSepVel = -separatingVel * restitution;
    float deltaVel = newSepVel - separatingVel;

    float totalInverseMass = body[0]->getInverseMass();
    if (body[1]) totalInverseMass += body[1]->getInverseMass();
    if (totalInverseMass <= 0) return;

    // Calculate impulse scalar
    float impulseScalar = deltaVel / totalInverseMass;
    Vector3 impulse = contactNormal * impulseScalar;

    //normal impulse
    body[0]->setVelocity(body[0]->getVelocity() + impulse * body[0]->getInverseMass());
    if (body[1]) {
        body[1]->setVelocity(body[1]->getVelocity() - impulse * body[1]->getInverseMass());
    }
    Vector3 relativeVel = body[0]->getVelocity();
    if (body[1]) relativeVel -= body[1]->getVelocity();

    // Remove normal component
    Vector3 lateralVel = relativeVel - contactNormal * relativeVel.dot(contactNormal);

    if (lateralVel.isZero()) return;

    // Normalize lateral direction
    lateralVel.normalize();

    // Calculate friction impulse magnitude
    float tangentialImpulseMag = -relativeVel.dot(lateralVel) / totalInverseMass;

    // Coulomb's law
    float maxFrictionImpulse = friction * impulseScalar;
    if (std::abs(tangentialImpulseMag) > maxFrictionImpulse) {
        tangentialImpulseMag = maxFrictionImpulse * (tangentialImpulseMag < 0 ? -1.0f : 1.0f);
    }

    // Apply friction impulse
    Vector3 frictionImpulse = lateralVel * tangentialImpulseMag;
    body[0]->setVelocity(body[0]->getVelocity() + frictionImpulse * body[0]->getInverseMass());
    if (body[1]) {
        body[1]->setVelocity(body[1]->getVelocity() - frictionImpulse * body[1]->getInverseMass());
    }
}