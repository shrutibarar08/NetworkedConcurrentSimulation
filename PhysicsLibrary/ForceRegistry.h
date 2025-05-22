#pragma once
#include <vector>
#include "ForceGenerator.h"
#include "RigidBody.h"

class  ForceRegistry
{
public:
    void Add(RigidBody* body, ForceGenerator* fg);
    void Remove(RigidBody* body, ForceGenerator* fg);
    void Clear();
    void UpdateForces(float duration) const;

protected:
    struct ForceRegistration
    {
        RigidBody* RigidBody;
        ForceGenerator* ForceGenerates;
    };

    std::vector<ForceRegistration> RegisteredForces;
};
