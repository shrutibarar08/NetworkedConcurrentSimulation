#include "pch.h"
#include "ContactResolver.h"

ContactResolver::ContactResolver(unsigned iterations) : iterations(iterations), iterationsUsed(0) {}


void ContactResolver::setIterations(unsigned newIterations)
{
	iterations = newIterations;
}

void ContactResolver::resolveContacts(Contact* contactArray, unsigned numContacts, float duration)
{
    iterationsUsed = 0;
    while (iterationsUsed < iterations) {
        float max = 0;
        int maxIndex = numContacts;

        // Find the contact with the largest closing velocity
        for (unsigned i = 0; i < numContacts; ++i) {
            float sepVel = contactArray[i].calculateSeparatingVelocity();
            if (sepVel < max && (sepVel < 0 || contactArray[i].penetration > 0)) {
                max = sepVel;
                maxIndex = i;
            }
        }

        if (maxIndex == numContacts) break; // All contacts resolved

        // Resolve this contact
        contactArray[maxIndex].resolve(duration);
        ++iterationsUsed;
    }
}
