#pragma once

#include <DirectXMath.h>
#include "RigidBody.h"

#include <atomic>
#include <memory>
#include <unordered_map>

struct Contact;

enum class ColliderType: uint8_t
{
    Cube,
    Sphere,
    Capsule,
};

enum class ColliderState: uint8_t
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

    void RegisterCollision(const ICollider* collider);

    // Access to parent rigid body
    virtual RigidBody* GetRigidBody() const { return m_RigidBody; }

    ColliderState GetColliderState();
    void SetColliderState(ColliderState state);
    virtual void SetScale(const DirectX::XMVECTOR& vector) = 0;
    virtual DirectX::XMVECTOR GetScale() const = 0;

    const char* ToString() const;

    // For type-safe down casting
    template<typename T>
    T* As() { return dynamic_cast<T*>(this); }

    template<typename T>
    const T* As() const { return dynamic_cast<const T*>(this); }

    template<typename T>
    constexpr const T& Min(const T& a, const T& b)
    {
        return (a < b) ? a : b;
    }

    const char* GetColliderTypeName() const;

    DirectX::XMMATRIX GetTransformationMatrix() const;
    void Update(float deltaTime);
    DirectX::XMMATRIX GetWorldMatrix() const;

    void SetReverseAware(bool flag) { m_ReverseAware = flag; }
    bool IsReverseAware() const { return m_ReverseAware; }

protected:
    bool m_ReverseAware{ false };
    ColliderState m_ColliderState = ColliderState::Dynamic;
    RigidBody* m_RigidBody;
    DirectX::XMMATRIX m_TransformationMatrix{};

    //~ Platform
    struct CollisionInfo
    {
        int hitCount = 0;
        float lastHitTime = 0.0f;
    } m_PlatformCollisionInfo{};

    float mTotalElapsedTime = 0.0f;
    int mRestThreshold{ 10 };
    float mRestTimeThreshold{ 2.f };
};
