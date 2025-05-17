#pragma once
#include "Vector3.h"
#include "RigidBody.h"

class Contact; // forward declaration

class Collider {
public:
    enum class Type {
        Sphere,
        Plane,
        Capsule,
        Box
    };

    virtual ~Collider() {}

    virtual Type getType() const = 0;
    virtual bool checkCollision(Collider* other, Contact& contact) const = 0;

    RigidBody* getBody() const { return body; }

protected:
    RigidBody* body;

    Collider(RigidBody* attachedBody) : body(attachedBody) {}
};
#include "Contact.h" // Add this include to resolve the incomplete type error
