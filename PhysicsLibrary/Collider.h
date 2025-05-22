#pragma once
#include <cstdint>
#include "RigidBody.h"

class Contact;

class Collider
{
public:
    enum class Type: uint8_t
	{
        SPHERE,
        PLANE,
        CAPSULE,
        BOX
    };

    Collider() = default;
    virtual ~Collider() = default;
    Collider(Collider&&) = default;
    Collider(const Collider&) = default;
    Collider& operator=(Collider&&) = default;
    Collider& operator=(const Collider&) = default;

    virtual Type GetType() const = 0;
	bool CheckCollision(Collider* other, Contact& contact) const;

    RigidBody* GetBody() const { return m_RigidBody; }

protected:
    Collider(RigidBody* attachedBody) : m_RigidBody(attachedBody) {}

    virtual bool CheckCollisionWithBox(Collider* other, Contact& contact) const = 0;
    virtual bool CheckCollisionWithSphere(Collider* other, Contact& contact) const = 0;
    virtual bool CheckCollisionWithPlane(Collider* other, Contact& contact) const = 0;
    virtual bool CheckCollisionWithCapsule(Collider* other, Contact& contact) const = 0;

protected:
    RigidBody* m_RigidBody;
};
#include "Contact.h"
