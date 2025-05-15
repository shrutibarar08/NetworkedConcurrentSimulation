#pragma once
#include <vector>
#include "ForceGenerator.h"
#include "Particle.h"

class ForceRegistry {
protected:
    struct ForceRegistration {
        Particle* particle;
        ForceGenerator* fg;
    };

    std::vector<ForceRegistration> registrations;

public:
    void add(Particle* p, ForceGenerator* fg);
    void remove(Particle* p, ForceGenerator* fg);
    void clear();
    void updateForces(float duration);
};
