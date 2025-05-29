#pragma once
#include <vector>
#include "ForceGenerator.h"
#include "ICollider.h"

#include <concurrent_queue.h>


class  ForceRegistry
{
public:
    void Add(ICollider* collider, ForceGenerator* fg);
    void Remove(ICollider* collider, ForceGenerator* fg);
    void Clear();
    void UpdateForces(float duration);

protected:
    struct ForceRegistration
    {
        ICollider* Collider;
        ForceGenerator* ForceGenerates;
    };

    Concurrency::concurrent_queue<ForceRegistration> RegisteredForces;
};
