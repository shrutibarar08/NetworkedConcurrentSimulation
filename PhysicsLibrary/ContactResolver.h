#pragma once
#include "Contact.h"

class ContactResolver
{
public:
    ContactResolver(unsigned iterations);
    void resolveContacts(Contact* contacts, unsigned numContacts, float duration) const;
    unsigned Iterations;

private:
    unsigned IterationsUsed;
};

