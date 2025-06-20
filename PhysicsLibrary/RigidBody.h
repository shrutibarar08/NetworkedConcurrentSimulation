#pragma once
#include <DirectXMath.h>

#include <windows.h>
#include "Quaternion.h"
#include "IntegrationType.h"


struct AtomicVector
{
    DirectX::XMVECTOR value = DirectX::XMVectorZero();
    std::atomic_flag lock = ATOMIC_FLAG_INIT;

    DirectX::XMVECTOR Get()
    {
        while (lock.test_and_set(std::memory_order_acquire)); // spin
        DirectX::XMVECTOR val = value;
        lock.clear(std::memory_order_release);
        return val;
    }

    void Set(DirectX::XMVECTOR v)
    {
        while (lock.test_and_set(std::memory_order_acquire)); // spin
        value = v;
        lock.clear(std::memory_order_release);
    }
};


class RigidBody
{
public:
    RigidBody();

    void CalculateDerivedData();

    void ClearAccumulators();
    void AddForce(const DirectX::XMVECTOR& force);
    void AddTorque(const DirectX::XMVECTOR& torque);
    void Integrate(float dt, IntegrationType type = IntegrationType::SemiImplicitEuler);
    DirectX::XMMATRIX GetTransformMatrix();

    // Setters
    void SetPosition(const DirectX::XMVECTOR& pos);
    void SetVelocity(const DirectX::XMVECTOR& vel);
    void SetAcceleration(const DirectX::XMVECTOR& acc);
    void SetOrientation(const Quaternion& q);
    void SetAngularVelocity(const DirectX::XMVECTOR& av);
    void SetMass(float mass);
    void SetInverseMass(float invMass);
    void SetLinearDamping(float d);
    void SetAngularDamping(float d);
    void SetInverseInertiaTensor(const DirectX::XMMATRIX& tensor);
    void SetDamping(float d);
    void SetElasticity(float e);
    void SetRestitution(float v);
    void SetFriction(float v);

    // Getters
    DirectX::XMVECTOR GetPosition();
    DirectX::XMVECTOR GetVelocity();
    DirectX::XMVECTOR GetAcceleration();
    DirectX::XMVECTOR GetAngularVelocity();
    Quaternion GetOrientation() const;
    float GetMass() const;
    float GetElasticity() const;
    float GetInverseMass() const;
    DirectX::XMMATRIX GetInverseInertiaTensor() const;
    DirectX::XMMATRIX GetInverseInertiaTensorWorld() const;
    bool HasFiniteMass() const;
    float GetDamping() const;
    float GetAngularDamping();
    float GetRestitution() const;
    float GetFriction() const;

    void SetRestingState(bool state);
    bool GetRestingState() const;

    void ConstrainVelocity(const DirectX::XMVECTOR& contactNormal);

    void SetAsPlatform(bool state);
    bool IsPlatform() const;

    void ComputeInverseInertiaTensorBox(float width, float height, float depth);
    void ComputeInverseInertiaTensorSphere(float radius);
    void ComputeInverseInertiaTensorCapsule(float radius, float height);

    void ApplyLinearImpulse(const DirectX::XMVECTOR& impulse);
    void ApplyAngularImpulse(const DirectX::XMVECTOR& impulse, const DirectX::XMVECTOR& contactVector);

private:
    void IntegrateEuler(float dt);
    void IntegrateSemiImplicitEuler(float dt);
    void IntegrateVerlet(float dt);

    void IntegrateAngular(float dt);
    void ApplyLinearDamping(DirectX::XMVECTOR& vel, float dt) const;

    void ResetVerletState(float delta);
    DirectX::XMVECTOR ClampVectorLength(DirectX::XMVECTOR vec, float maxLength);

private:
    bool m_VerletNeedsReset{ false };
    std::atomic<bool> m_Platform{ false };
    std::atomic<bool> m_Resting{ false };
    std::atomic<float> InverseMass{ 10.f };
    std::atomic<float> m_LinearDamping{ 0.75f };
    std::atomic<float> m_Elastic{ 0.56f };
    std::atomic<float> m_Restitution{ 0.35f };
    std::atomic<float> m_Friction{ 0.38f };
    std::atomic<float> AngularDamping{ 0.39f };


    Quaternion Orientation;
    AtomicVector Position;
    AtomicVector m_LastPosition;
    AtomicVector Velocity;
    AtomicVector Acceleration;
    AtomicVector ForceAccum;
    AtomicVector AngularVelocity;
    AtomicVector TorqueAccum;

    DirectX::XMMATRIX m_InverseInertiaTensorLocal;
    DirectX::XMMATRIX InverseInertiaTensorWorld;
    DirectX::XMMATRIX TransformMatrix;
};
