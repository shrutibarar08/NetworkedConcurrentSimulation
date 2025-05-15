#include "pch.h"
#include "ContactResolver.h"

ContactResolver::ContactResolver(unsigned iterations)
    : iterations(iterations), iterationsUsed(0) {
}

void ContactResolver::resolveContacts(Contact* contacts, unsigned numContacts, float duration) {
    iterationsUsed = 0;
    while (iterationsUsed < iterations) {
        float max = 0;
        unsigned maxIndex = numContacts;
        for (unsigned i = 0; i < numContacts; i++) {
            float sepVel = contacts[i].calculateSeparatingVelocity();
            if (sepVel < max) {
                max = sepVel;
                maxIndex = i;
            }
        }

        if (maxIndex == numContacts) break;

        contacts[maxIndex].resolve(duration);
        iterationsUsed++;
    }
}
