#pragma once
#include "Vector3.h"
#include "Contact.h"

class Collider {
public:
    enum class Type {
        Sphere,
        Plane
    };

    virtual ~Collider() = default;

    virtual Type getType() const = 0;
    virtual Vector3 getCenter() const = 0;    // For Sphere
    virtual float getRadius() const = 0;      // For Sphere
    virtual Vector3 getNormal() const { return Vector3(); } // For Plane
    virtual float getOffset() const { return 0.0f; }         // For Plane
    virtual bool checkCollision(Collider* other, Contact& contact) const = 0;

};

