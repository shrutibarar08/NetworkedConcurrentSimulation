#pragma once
#include "Contact.h"

class ContactResolver {
<<<<<<< Updated upstream

public:
    explicit ContactResolver(unsigned iterations);
    void resolveContacts(Contact* contactArray, unsigned numContacts, float duration);

private:
    unsigned iterations;
    unsigned iterationsUsed;};
=======
    unsigned iterations;
public:
    ContactResolver(unsigned iterations);
    void resolveContacts(Contact* contacts, unsigned numContacts, float duration);
};

// ContactResolver.cpp
#include "ContactResolver.h"
#include <cfloat>

ContactResolver::ContactResolver(unsigned iterations)
    : iterations(iterations) {
}

void ContactResolver::resolveContacts(Contact* contacts, unsigned numContacts, float duration) {
    for (unsigned i = 0; i < iterations; ++i) {
        float maxSepVel = -FLT_MAX;
        int maxIndex = numContacts;

        for (unsigned j = 0; j < numContacts; ++j) {
            float sepVel = contacts[j].calculateSeparatingVelocity();
            if (sepVel < maxSepVel && (contacts[j].penetration > 0 || sepVel < 0)) {
                maxSepVel = sepVel;
                maxIndex = j;
            }
        }

        if (maxIndex == numContacts) break;

        contacts[maxIndex].resolve(duration);
    }
}
>>>>>>> Stashed changes
