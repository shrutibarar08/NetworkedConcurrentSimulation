#include "pch.h"
#include "ForceRegistry.h"
#include <algorithm>

void ForceRegistry::add(Particle* p, ForceGenerator* fg) {
    registrations.push_back({ p, fg });
}

void ForceRegistry::remove(Particle* p, ForceGenerator* fg) {
    registrations.erase(
        std::remove_if(registrations.begin(), registrations.end(),
            [=](const ForceRegistration& reg) {
                return reg.particle == p && reg.fg == fg;
            }),
        registrations.end()
    );
}

void ForceRegistry::clear() {
    registrations.clear();
}

void ForceRegistry::updateForces(float duration) {
    for (auto& reg : registrations) {
        reg.fg->updateForce(reg.particle, duration);
    }
}

