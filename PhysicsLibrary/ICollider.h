#pragma once

#include <DirectXMath.h>
#include "RigidBody.h"

#include <atomic>
#include <memory>

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
    virtual void SetScale(const DirectX::XMVECTOR& vector) = 0;
    virtual DirectX::XMVECTOR GetScale() const = 0;

    const char* ToString() const;

    // For type-safe down casting
    template<typename T>
    T* As() { return dynamic_cast<T*>(this); }

    template<typename T>
    const T* As() const { return dynamic_cast<const T*>(this); }

    const char* GetColliderTypeName() const;

    DirectX::XMMATRIX GetTransformationMatrix() const;
    void Update();

protected:
    ColliderSate m_ColliderState = ColliderSate::Dynamic;
    RigidBody* m_RigidBody;
    DirectX::XMMATRIX m_TransformationMatrix{};
};
