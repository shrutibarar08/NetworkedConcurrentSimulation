#include "pch.h"
#include "ContactResolver.h"

ContactResolver::ContactResolver(unsigned iterations)
    : iterations(iterations), iterationsUsed(0) {
}

void ContactResolver::resolveContacts(Contact* contacts, unsigned numContacts, float duration) {
    iterationsUsed = 0;
    while (iterationsUsed < iterations) {
        // Find the contact with the largest closing velocity
        float max = 0;
        unsigned maxIndex = numContacts;

        for (unsigned i = 0; i < numContacts; ++i) {
            float sepVel = contacts[i].calculateSeparatingVelocity();
            if (sepVel < max && (contacts[i].penetration > 0 || sepVel < 0)) {
                max = sepVel;
                maxIndex = i;
            }
        }

        // No contacts need resolving
        if (maxIndex == numContacts) break;

        // Resolve this contact
        contacts[maxIndex].resolve(duration);

        ++iterationsUsed;
    }
}
