#include "pch.h"
#include "pch.h"
#include "RigidBody.h"
#include <cmath>
#include <iostream>
#include <string>

RigidBody::RigidBody()
    : Position(), Velocity(), Acceleration(), ForceAccum(),
    InverseMass(1.0f), m_LinearDamping(0.98f),
    Orientation(1, 0, 0, 0), AngularVelocity(), TorqueAccum(),
    AngularDamping(0.9f) {
    InverseInertiaTensor = DirectX::XMMatrixIdentity();
    CalculateDerivedData();
}

void RigidBody::Integrate(float duration)
{
    if (InverseMass <= 0.0f) return;

    AcquireSRWLockExclusive(&m_Lock);

    // === Linear motion ===
    Position = DirectX::XMVectorAdd(Position, DirectX::XMVectorScale(Velocity, duration));
    DirectX::XMVECTOR linearAcc = DirectX::XMVectorAdd(
        Acceleration,
        DirectX::XMVectorScale(ForceAccum, InverseMass)
    );
    Velocity = DirectX::XMVectorAdd(Velocity, DirectX::XMVectorScale(linearAcc, duration));
    Velocity = DirectX::XMVectorScale(Velocity, std::pow(m_LinearDamping, duration));

    // === Angular motion ===
    DirectX::XMVECTOR angularAcc = DirectX::XMVector3Transform(TorqueAccum, InverseInertiaTensorWorld);
    AngularVelocity = DirectX::XMVectorAdd(AngularVelocity, DirectX::XMVectorScale(angularAcc, duration));
    AngularVelocity = DirectX::XMVectorScale(AngularVelocity, std::pow(AngularDamping, duration));

    // === Orientation ===
    float x = DirectX::XMVectorGetX(AngularVelocity);
    float y = DirectX::XMVectorGetY(AngularVelocity);
    float z = DirectX::XMVectorGetZ(AngularVelocity);

    Quaternion deltaRot(0.0f, x, y, z);
    deltaRot = deltaRot * Orientation;
    Orientation += deltaRot * 0.5f * duration;
    Orientation.Normalize();

    ReleaseSRWLockExclusive(&m_Lock);

    CalculateDerivedData();
    ClearAccumulators();

}

void RigidBody::AddForce(const DirectX::XMVECTOR& force)
{
    AcquireSRWLockExclusive(&m_Lock);
    ForceAccum = DirectX::XMVectorAdd(ForceAccum, force);
    ReleaseSRWLockExclusive(&m_Lock);
}

void RigidBody::AddTorque(const DirectX::XMVECTOR& torque)
{
    AcquireSRWLockExclusive(&m_Lock);
    TorqueAccum = DirectX::XMVectorAdd(TorqueAccum, torque);
    ReleaseSRWLockExclusive(&m_Lock);
}

void RigidBody::Integrate(float dt, IntegrationType type)
{
    AcquireSRWLockExclusive(&m_Lock);
    if (InverseMass <= 0.0f) return;

    DirectX::XMVECTOR acceleration = DirectX::XMVectorAdd(
        Acceleration,
        DirectX::XMVectorScale(ForceAccum, InverseMass)
    );

    switch (type)
    {
    case IntegrationType::Euler:
    {
        Position = DirectX::XMVectorAdd(Position, DirectX::XMVectorScale(Velocity, dt));
        Velocity = DirectX::XMVectorAdd(Velocity, DirectX::XMVectorScale(acceleration, dt));
        break;
    }
    case IntegrationType::SemiImplicitEuler:
    {
        Velocity = DirectX::XMVectorAdd(Velocity, DirectX::XMVectorScale(acceleration, dt));
        Position = DirectX::XMVectorAdd(Position, DirectX::XMVectorScale(Velocity, dt));
        break;
    }
    case IntegrationType::Verlet:
    {
        DirectX::XMVECTOR posDelta = DirectX::XMVectorSubtract(Position, m_LastPosition);
        DirectX::XMVECTOR accelTerm = DirectX::XMVectorScale(acceleration, dt * dt);
        DirectX::XMVECTOR newPosition = DirectX::XMVectorAdd(Position, DirectX::XMVectorAdd(posDelta, accelTerm));

        m_LastPosition = Position;
        Position = newPosition;

        break;
    }
    }
    // Apply damping
    Velocity = DirectX::XMVectorScale(Velocity, std::pow(m_LinearDamping, dt));

    // Angular motion
    DirectX::XMVECTOR angularAcc = DirectX::XMVector3Transform(TorqueAccum, InverseInertiaTensor);
    AngularVelocity = DirectX::XMVectorAdd(AngularVelocity, DirectX::XMVectorScale(angularAcc, dt));
    AngularVelocity = DirectX::XMVectorScale(AngularVelocity, std::pow(AngularDamping, dt));

    // Update orientation using angular velocity

    Orientation.AddScaledVector(AngularVelocity, dt);
    Orientation.Normalize();

    if (DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(Velocity)) < 1e-5f) Velocity = DirectX::XMVectorZero();

    ReleaseSRWLockExclusive(&m_Lock);

    ClearAccumulators();
}

DirectX::XMMATRIX RigidBody::GetTransformMatrix() const
{
    using namespace DirectX;

    AcquireSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));

    XMMATRIX scale = XMMatrixScalingFromVector(Scale);
    XMMATRIX rotation = Orientation.ToRotationMatrix();
    XMMATRIX translation = XMMatrixTranslationFromVector(Position);

    XMMATRIX result = scale * rotation * translation;

    ReleaseSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));

    return result;
}

void RigidBody::CalculateDerivedData()
{
    using namespace DirectX;

    AcquireSRWLockExclusive(&m_Lock);

    Orientation.Normalize(); // Prevent drift

    XMMATRIX rotMatrix = XMMatrixRotationQuaternion(Orientation.ToXmVector());
    XMMATRIX translation = XMMatrixTranslationFromVector(Position);

    TransformMatrix = rotMatrix * translation; // Full local-to-world matrix

    XMMATRIX rotTranspose = XMMatrixTranspose(rotMatrix);
    InverseInertiaTensorWorld = XMMatrixMultiply(
        XMMatrixMultiply(rotMatrix, InverseInertiaTensor),
        rotTranspose
    );

    ReleaseSRWLockExclusive(&m_Lock);
}

void RigidBody::ClearAccumulators()
{
    AcquireSRWLockExclusive(&m_Lock);

    ForceAccum = {};
    TorqueAccum = {};

    ReleaseSRWLockExclusive(&m_Lock);
}

// Setters
void RigidBody::SetPosition(const DirectX::XMVECTOR& pos)
{
    AcquireSRWLockExclusive(&m_Lock);
    Position = pos;
    ReleaseSRWLockExclusive(&m_Lock);
}

void RigidBody::SetBodyScale(const DirectX::XMVECTOR& scale)
{
    AcquireSRWLockExclusive(&m_Lock);
    Scale = scale;
    ReleaseSRWLockExclusive(&m_Lock);
}

void RigidBody::SetVelocity(const DirectX::XMVECTOR& vel)
{
    using namespace DirectX;

    // Store old velocity
    XMFLOAT3 oldVel;
    XMStoreFloat3(&oldVel, Velocity);

    XMFLOAT3 newVel;
    XMStoreFloat3(&newVel, vel);

    std::cout << "[SetVelocity] From ("
        << oldVel.x << ", " << oldVel.y << ", " << oldVel.z << ") "
        << "To ("
        << newVel.x << ", " << newVel.y << ", " << newVel.z << ")\n";

    AcquireSRWLockExclusive(&m_Lock);
    Velocity = vel;
    ReleaseSRWLockExclusive(&m_Lock);
}

void RigidBody::SetDamping(float d)
{
    AcquireSRWLockExclusive(&m_Lock);
    m_LinearDamping = d;
    ReleaseSRWLockExclusive(&m_Lock);
}

void RigidBody::SetElasticity(float e)
{
    AcquireSRWLockExclusive(&m_Lock);
    m_Elastic = e;
    ReleaseSRWLockExclusive(&m_Lock);
}

void RigidBody::SetRestitution(float v)
{
    AcquireSRWLockExclusive(&m_Lock);
    m_Elastic = v;
    ReleaseSRWLockExclusive(&m_Lock);
}

void RigidBody::SetFriction(float v)
{
    AcquireSRWLockExclusive(&m_Lock);
    m_Elastic = v;
    ReleaseSRWLockExclusive(&m_Lock);
}

void RigidBody::SetAcceleration(const DirectX::XMVECTOR& acc)
{
    AcquireSRWLockExclusive(&m_Lock);
    Acceleration = acc;
    ReleaseSRWLockExclusive(&m_Lock);
}

void RigidBody::SetOrientation(const Quaternion& q)
{
    AcquireSRWLockExclusive(&m_Lock);
    Orientation = q;
    Orientation.Normalize();
    ReleaseSRWLockExclusive(&m_Lock);
}

void RigidBody::SetAngularVelocity(const DirectX::XMVECTOR& av)
{
    AcquireSRWLockExclusive(&m_Lock);
    AngularVelocity = av;
    ReleaseSRWLockExclusive(&m_Lock);
}

void RigidBody::SetMass(float mass)
{
    AcquireSRWLockExclusive(&m_Lock);
    InverseMass = (mass > 0.0f) ? 1.0f / mass : 0.0f;
    ReleaseSRWLockExclusive(&m_Lock);
}

void RigidBody::SetInverseMass(float invMass)
{
    AcquireSRWLockExclusive(&m_Lock);
    InverseMass = invMass;
    ReleaseSRWLockExclusive(&m_Lock);
}

void RigidBody::SetLinearDamping(float d)
{
    AcquireSRWLockExclusive(&m_Lock);
    m_LinearDamping = d;
    ReleaseSRWLockExclusive(&m_Lock);
}

void RigidBody::SetAngularDamping(float d)
{
    AcquireSRWLockExclusive(&m_Lock);
    AngularDamping = d;
    ReleaseSRWLockExclusive(&m_Lock);
}

void RigidBody::SetInverseInertiaTensor(const DirectX::XMMATRIX& tensor)
{
    AcquireSRWLockExclusive(&m_Lock);
    InverseInertiaTensor = tensor;
    ReleaseSRWLockExclusive(&m_Lock);
}

// Getters
DirectX::XMVECTOR RigidBody::GetPosition() const
{
    AcquireSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));
    auto result = Position;
    ReleaseSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));
    return result;
}

DirectX::XMVECTOR RigidBody::GetBodyScale() const
{
    AcquireSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));
    auto result = Scale;
    ReleaseSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));
    return result;
}

DirectX::XMVECTOR RigidBody::GetVelocity() const
{
    AcquireSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));
    auto result = Velocity;
    ReleaseSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));
    return result;
}

DirectX::XMVECTOR RigidBody::GetAcceleration() const
{
    AcquireSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));
    auto result = Acceleration;
    ReleaseSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));
    return result;
}

DirectX::XMVECTOR RigidBody::GetAngularVelocity() const
{
    AcquireSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));
    auto result = AngularVelocity;
    ReleaseSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));
    return result;
}

Quaternion RigidBody::GetOrientation() const
{
    AcquireSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));
    auto result = Orientation;
    ReleaseSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));
    return result;
}

float RigidBody::GetMass() const
{
    AcquireSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));
    float result = (InverseMass > 0.0f) ? 1.0f / InverseMass : INFINITY;
    ReleaseSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));
    return result;
}

float RigidBody::GetElasticity() const
{
    AcquireSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));
    float result = m_Elastic;
    ReleaseSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));
    return result;
}

float RigidBody::GetInverseMass() const
{
    AcquireSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));
    float result = InverseMass;
    ReleaseSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));
    return result;
}

DirectX::XMMATRIX RigidBody::GetInverseInertiaTensor() const
{
    AcquireSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));
    auto result = InverseInertiaTensor;
    ReleaseSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));
    return result;
}

bool RigidBody::HasFiniteMass() const
{
    AcquireSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));
    bool result = (InverseMass > 0.0f);
    ReleaseSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));
    return result;
}

float RigidBody::GetDamping() const
{
    AcquireSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));
    float result = m_LinearDamping;
    ReleaseSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));
    return result;
}

float RigidBody::GetRestitution() const
{
    AcquireSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));
    float result = m_Restitution;
    ReleaseSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));
    return result;
}

float RigidBody::GetFriction() const
{
    AcquireSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));
    float result = m_Friction;
    ReleaseSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));
    return result;
}

void RigidBody::SetRestingState(bool state)
{
    AcquireSRWLockExclusive(&m_Lock);
    m_Resting = state;
    ReleaseSRWLockExclusive(&m_Lock);
}

bool RigidBody::GetRestingState() const
{
    AcquireSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));
    bool result = m_Resting;
    ReleaseSRWLockShared(const_cast<SRWLOCK*>(&m_Lock));
    return result;
}

void RigidBody::ConstrainVelocity(const DirectX::XMVECTOR& contactNormal)
{
	DirectX::XMVECTOR v = GetVelocity();
    float vIntoSurface = DirectX::XMVectorGetX(DirectX::XMVector3Dot(v, contactNormal));
    if (vIntoSurface < 0.0f)
    {
	    DirectX::XMVECTOR correction = DirectX::XMVectorScale(contactNormal, vIntoSurface);
        SetVelocity(DirectX::XMVectorSubtract(v, correction));
    }
}
