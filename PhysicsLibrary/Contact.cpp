#include "pch.h"
#include "Contact.h"

Contact::Contact()
    : restitution(0.0f),
    friction(0.0f),
    penetration(0.0f),
    contactPoint(Vector3()),
    contactNormal(Vector3())
{
    body[0] = nullptr;
    body[1] = nullptr;
}

float Contact::calculateSeparatingVelocity() const
{
    Vector3 relativeVelocity = body[0]->getVelocity();
    if (body[1]) {
        relativeVelocity -= body[1]->getVelocity();
    }
    return relativeVelocity.dot(contactNormal);
}

void Contact::resolve(float duration)
{
    float separatingVelocity = calculateSeparatingVelocity();
    if (separatingVelocity > 0) return;

    float newSepVelocity = -separatingVelocity * restitution;
    float deltaVelocity = newSepVelocity - separatingVelocity;

    float totalInverseMass = body[0]->getInverseMass();
    if (body[1]) totalInverseMass += body[1]->getInverseMass();
    if (totalInverseMass <= 0.0f) return;

    float impulse = deltaVelocity / totalInverseMass;
    Vector3 impulsePerIMass = contactNormal * impulse;

    body[0]->setVelocity(body[0]->getVelocity() + impulsePerIMass * body[0]->getInverseMass());
    if (body[1]) {
        body[1]->setVelocity(body[1]->getVelocity() - impulsePerIMass * body[1]->getInverseMass());
    }
}
