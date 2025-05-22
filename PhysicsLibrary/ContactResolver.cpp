#include "pch.h"
#include "ContactResolver.h"
#include <cfloat>

ContactResolver::ContactResolver(unsigned iterations)
    : IterationsUsed(0), Iterations(iterations)
{}

void ContactResolver::resolveContacts(Contact* contacts,
    unsigned numContacts, float duration) const
{
    for (unsigned i = 0; i < Iterations; ++i)
    {
        float maxSepVel = -FLT_MAX;
        unsigned int maxIndex = numContacts;

        for (unsigned j = 0; j < numContacts; ++j)
        {
            float sepVel = contacts[j].CalculateSeparatingVelocity();
            if (sepVel < maxSepVel && (contacts[j].Penetration > 0 || sepVel < 0))
            {
                maxSepVel = sepVel;
                maxIndex = j;
            }
        }

        if (maxIndex == numContacts) break;
        contacts[maxIndex].Resolve(duration);
    }
}
