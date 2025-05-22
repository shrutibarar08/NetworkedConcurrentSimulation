#include "pch.h"
#include "ForceRegistry.h"
#include <algorithm>



void ForceRegistry::Add(RigidBody* body, ForceGenerator* fg)
{
    RegisteredForces.push_back({body,fg});
}

void ForceRegistry::Remove(RigidBody* body, ForceGenerator* fg)
{
    RegisteredForces.erase(
        std::remove_if(RegisteredForces.begin(), RegisteredForces.end(),
            [&](const ForceRegistration& reg)
            {
                return reg.RigidBody == body && reg.ForceGenerates == fg;
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
        if (reg.RigidBody)
        {
            reg.ForceGenerates->UpdateForce(reg.RigidBody, duration);
        }
    }
}
