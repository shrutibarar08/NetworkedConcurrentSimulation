#include "pch.h"
#include "ContactResolver.h"
#include <cfloat>

ContactResolver::ContactResolver(unsigned iterations)
    : iterationsUsed(0), iterations(iterations)
{}

void ContactResolver::resolveContacts(Contact* contacts, unsigned numContacts, float duration) const
{
    for (unsigned i = 0; i < iterations; ++i)
    {
        float maxSepVel = -FLT_MAX;
        unsigned int maxIndex = numContacts;

        for (unsigned j = 0; j < numContacts; ++j)
        {
            float sepVel = contacts[j].calculateSeparatingVelocity();
            if (sepVel < maxSepVel && (contacts[j].penetration > 0 || sepVel < 0))
            {
                maxSepVel = sepVel;
                maxIndex = j;
            }
        }

        if (maxIndex == numContacts) break;
        contacts[maxIndex].resolve(duration);
    }
}
