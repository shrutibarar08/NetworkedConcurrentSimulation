#pragma once

#include <DirectXMath.h>
#include "RigidBody.h"

struct Contact;

enum class ColliderType: uint8_t
{
    Cube,
    Sphere,
    Capsule,
};

enum class ColliderSate: uint8_t
{
	Dynamic,
    Static,
    Resting
};

class ICollider
{
public:
    ICollider(RigidBody* attachBody);
    virtual ~ICollider() = default;

    // Collision interface
    virtual bool CheckCollision(ICollider* other, Contact& outContact) = 0;
    virtual ColliderType GetColliderType() const = 0;

    // Access to parent rigid body
    virtual RigidBody* GetRigidBody() const { return m_RigidBody; }

    ColliderSate GetColliderState();
    void SetColliderState(ColliderSate state);

    // For type-safe down casting
    template<typename T>
    T* As() { return dynamic_cast<T*>(this); }

    template<typename T>
    const T* As() const { return dynamic_cast<const T*>(this); }

    const char* GetColliderTypeName() const;

protected:
    SRWLOCK m_Lock{ SRWLOCK_INIT };
    ColliderSate m_ColliderState = ColliderSate::Dynamic;
    RigidBody* m_RigidBody;
};
