#pragma once
#include "Contact.h"

class ContactResolver {

public:
    explicit ContactResolver(unsigned iterations);
    void resolveContacts(Contact* contactArray, unsigned numContacts, float duration);

private:
    unsigned iterations;
    unsigned iterationsUsed;};