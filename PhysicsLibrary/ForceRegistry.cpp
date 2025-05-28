#include "pch.h"
#include "ForceRegistry.h"
#include <algorithm>



void ForceRegistry::Add(ICollider* collider, ForceGenerator* fg)
{
    RegisteredForces.push_back({ collider,fg });
}

void ForceRegistry::Remove(ICollider* collider, ForceGenerator* fg)
{
    RegisteredForces.erase(
        std::remove_if(RegisteredForces.begin(), RegisteredForces.end(),
            [&](const ForceRegistration& reg)
            {
                return reg.Collider == collider && reg.ForceGenerates == fg;
            }),
        RegisteredForces.end()
    );
}

void ForceRegistry::Clear()
{
    RegisteredForces.clear();
}

void ForceRegistry::UpdateForces(float duration) const
{
    for (auto& reg : RegisteredForces)
    {
        if (reg.Collider)
        {
            reg.ForceGenerates->UpdateForce(reg.Collider, duration);
        }
    }
}
