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
    virtual void SetScale(const DirectX::XMVECTOR& vector) = 0;
    virtual DirectX::XMVECTOR GetScale() const = 0;

    // For type-safe down casting
    template<typename T>
    T* As() { return dynamic_cast<T*>(this); }

    template<typename T>
    const T* As() const { return dynamic_cast<const T*>(this); }

    const char* GetColliderTypeName() const;

    ICollider* LastHitCollider() const { return m_LastHitCollider; }
    void SetLastHitCollider(ICollider* collider);

    int GetHitCount(ICollider* collider, float totalTime);
    bool IsLastHitResolved() const { return !m_LastHitResolved; }
    void SetLastHitResolved(bool val) { m_LastHitResolved = true; }

    DirectX::XMMATRIX GetTransformationMatrix() const;
    void Update();

protected:
    SRWLOCK m_Lock{ SRWLOCK_INIT };
    ColliderSate m_ColliderState = ColliderSate::Dynamic;
    RigidBody* m_RigidBody;

    float m_LastHitTime{ 0.0f };
    ICollider* m_LastHitCollider{ nullptr };
    int m_LastHitColliderCounts{ 0 };
    bool m_LastHitResolved{ false };
    DirectX::XMMATRIX m_TransformationMatrix{};
};
