#pragma once
#include <vector>
#include "ForceGenerator.h"
#include "ICollider.h"

class  ForceRegistry
{
public:
    void Add(ICollider* collider, ForceGenerator* fg);
    void Remove(ICollider* collider, ForceGenerator* fg);
    void Clear();
    void UpdateForces(float duration) const;

protected:
    struct ForceRegistration
    {
        ICollider* Collider;
        ForceGenerator* ForceGenerates;
    };

    std::vector<ForceRegistration> RegisteredForces;
};
