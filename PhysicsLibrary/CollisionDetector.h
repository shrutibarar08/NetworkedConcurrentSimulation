#pragma once
#include "SphereCollider.h"
#include "PlaneCollider.h"
#include "Contact.h"

class CollisionDetector {
public:
    static bool detectSphereSphere(const SphereCollider& a, const SphereCollider& b, Contact& contactOut);
    static bool detectSpherePlane(const SphereCollider& sphere, const PlaneCollider& plane, Contact& contactOut);

    static bool dispatch(Collider* a, Collider* b, Contact& contactOut);
};
