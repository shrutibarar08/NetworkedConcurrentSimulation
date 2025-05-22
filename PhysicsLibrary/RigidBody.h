#pragma once
#include <DirectXMath.h>
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

    // Getters
    DirectX::XMVECTOR GetPosition() const;
    DirectX::XMVECTOR GetVelocity() const;
    DirectX::XMVECTOR GetAcceleration() const;
    DirectX::XMVECTOR GetAngularVelocity() const;
    Quaternion GetOrientation() const;
    float GetMass() const;
    float GetInverseMass() const;
    DirectX::XMMATRIX GetInverseInertiaTensor() const;
    bool HasFiniteMass() const;
    float GetDamping() const;

    DirectX::XMVECTOR Position;
    DirectX::XMVECTOR Velocity;
    DirectX::XMVECTOR Acceleration;

private:

    DirectX::XMVECTOR ForceAccum;
    float InverseMass;
    float m_LinearDamping;

    Quaternion Orientation;
    DirectX::XMVECTOR AngularVelocity;
    DirectX::XMVECTOR TorqueAccum;
    float AngularDamping;

    DirectX::XMMATRIX InverseInertiaTensor;
    DirectX::XMMATRIX InverseInertiaTensorWorld;
    DirectX::XMMATRIX TransformMatrix;
};
