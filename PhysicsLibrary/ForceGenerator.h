#pragma once
class Particle;
class ForceGenerator {
public:
    virtual void updateForce(Particle* particle, float duration) = 0;
    virtual ~ForceGenerator() {}};
