#pragma once
#include "Contact.h"

class ContactResolver {
private:
    unsigned iterationsUsed;
public:
    ContactResolver(unsigned iterations);
    void resolveContacts(Contact* contacts, unsigned numContacts, float duration) const;
    unsigned iterations;
};

