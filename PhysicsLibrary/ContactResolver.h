#pragma once
#include "Contact.h"

class ContactResolver
{
public:
    ContactResolver(unsigned iterations);

    void setIterations(unsigned iterations);
    void resolveContacts(Contact* contactArray, unsigned numContacts, float duration);

private:
    unsigned iterations;
    unsigned iterationsUsed;
};

