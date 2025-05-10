#pragma once
#include "Particle.h"

class Collider {
public:
    enum class Type { Sphere, Plane, Box, Capsule };

    virtual Type getType() const = 0;
    virtual Particle* getParticle() const = 0;  
    virtual ~Collider() = default;
};
