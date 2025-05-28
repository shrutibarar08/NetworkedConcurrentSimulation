#pragma once
#include <DirectXMath.h>

#include <windows.h>
#include "Quaternion.h"
#include "IntegrationType.h"

class RigidBody
{
public:
    RigidBody();

    void Integrate(float duration);
    void CalculateDerivedData();

    void ClearAccumulators();
    void AddForce(const DirectX::XMVECTOR& force);
    void AddTorque(const DirectX::XMVECTOR& torque);
    void Integrate(float dt, IntegrationType type = IntegrationType::SemiImplicitEuler);
    DirectX::XMMATRIX GetTransformMatrix() const;

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
    DirectX::XMVECTOR GetPosition() const;
    DirectX::XMVECTOR GetVelocity() const;
    DirectX::XMVECTOR GetAcceleration() const;
    DirectX::XMVECTOR GetAngularVelocity() const;
    Quaternion GetOrientation() const;
    float GetMass() const;
    float GetElasticity() const;
    float GetInverseMass() const;
    DirectX::XMMATRIX GetInverseInertiaTensor() const;
    bool HasFiniteMass() const;
    float GetDamping() const;
    float GetAngularDamping();
    float GetRestitution() const;
    float GetFriction() const;

    void SetRestingState(bool state);
    bool GetRestingState() const;

    void ConstrainVelocity(const DirectX::XMVECTOR& contactNormal);
private:
    bool m_Resting{ false };
    DirectX::XMVECTOR Position;
    DirectX::XMVECTOR m_LastPosition{};
    DirectX::XMVECTOR Velocity;
    DirectX::XMVECTOR Acceleration;

    SRWLOCK m_Lock{ SRWLOCK_INIT };

    DirectX::XMVECTOR ForceAccum;
    float InverseMass{ 10.f };
    float m_LinearDamping{ 0.75 };
    float m_Elastic{ 0.56f };
    float m_Restitution{ 0.35f };
    float m_Friction{ 0.38f };

    Quaternion Orientation;
    DirectX::XMVECTOR AngularVelocity;
    DirectX::XMVECTOR TorqueAccum;
    float AngularDamping{ 0.39f };

    DirectX::XMMATRIX InverseInertiaTensor;
    DirectX::XMMATRIX InverseInertiaTensorWorld;
    DirectX::XMMATRIX TransformMatrix;
};
