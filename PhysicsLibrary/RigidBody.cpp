#include "pch.h"
#include "pch.h"
#include "RigidBody.h"
#include <cmath>

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

    // === Linear motion ===
    Position = DirectX::XMVectorAdd(Position, DirectX::XMVectorScale(Velocity, duration));

    DirectX::XMVECTOR linearAcc = DirectX::XMVectorAdd(
        Acceleration,
        DirectX::XMVectorScale(ForceAccum, InverseMass)
    );

    Velocity = DirectX::XMVectorAdd(Velocity, DirectX::XMVectorScale(linearAcc, duration));
    Velocity = DirectX::XMVectorScale(Velocity, std::pow(m_LinearDamping, duration));

    // === Angular motion ===
    // Use: angularAcc = InverseInertiaTensorWorld * TorqueAccum (in vector math)
    DirectX::XMVECTOR angularAcc = DirectX::XMVector3Transform(
        TorqueAccum,
        InverseInertiaTensorWorld
    );

    AngularVelocity = DirectX::XMVectorAdd(AngularVelocity, DirectX::XMVectorScale(angularAcc, duration));
    AngularVelocity = DirectX::XMVectorScale(AngularVelocity, std::pow(AngularDamping, duration));

    // === Orientation (Quaternion integration) ===
    float x = DirectX::XMVectorGetX(AngularVelocity);
    float y = DirectX::XMVectorGetY(AngularVelocity);
    float z = DirectX::XMVectorGetZ(AngularVelocity);

    Quaternion deltaRot(0.0f, x, y, z);
    deltaRot = deltaRot * Orientation;
    Orientation += deltaRot * 0.5f * duration;
    Orientation.Normalize();

    // === Final updates ===
    CalculateDerivedData();
    ClearAccumulators();
}

void RigidBody::AddForce(const DirectX::XMVECTOR& force)
{
    ForceAccum = DirectX::XMVectorAdd(ForceAccum, force);
}

void RigidBody::AddTorque(const DirectX::XMVECTOR& torque)
{
    TorqueAccum = DirectX::XMVectorAdd(TorqueAccum, torque);
}

void RigidBody::Integrate(float dt, IntegrationType type)
{
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
        static DirectX::XMVECTOR lastPosition = Position; // not thread-safe or per-object safe
        DirectX::XMVECTOR posDelta = DirectX::XMVectorSubtract(Position, lastPosition);
        DirectX::XMVECTOR accelTerm = DirectX::XMVectorScale(acceleration, dt * dt);
        DirectX::XMVECTOR newPosition = DirectX::XMVectorAdd(Position, DirectX::XMVectorAdd(posDelta, accelTerm));
        lastPosition = Position;
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

    ClearAccumulators();
}

void RigidBody::CalculateDerivedData()
{
    // Normalize orientation to prevent drift
    Orientation.Normalize();

    // Create a rotation matrix from the quaternion
    DirectX::XMMATRIX rotMatrix = DirectX::XMMatrixRotationQuaternion(Orientation.ToXmVector());

    // Update the transform matrix (rotation only; add translation if needed)
    TransformMatrix = rotMatrix;

    // Transform the local inverse inertia tensor into world space
    DirectX::XMMATRIX rotTranspose = DirectX::XMMatrixTranspose(rotMatrix);
    InverseInertiaTensorWorld = DirectX::XMMatrixMultiply(
        DirectX::XMMatrixMultiply(rotMatrix, InverseInertiaTensor),
        rotTranspose
    );
}

void RigidBody::ClearAccumulators()
{
    ForceAccum = {};
    TorqueAccum = {};
}

// Setters
void RigidBody::SetPosition(const DirectX::XMVECTOR& pos)
{ 
    Position = pos;
}

void RigidBody::SetVelocity(const DirectX::XMVECTOR& vel)
{
    Velocity = vel; 
}

void RigidBody::SetDamping(float d)
{
    m_LinearDamping = d;
}

void RigidBody::SetAcceleration(const DirectX::XMVECTOR& acc)
{
    Acceleration = acc; 
}

void RigidBody::SetOrientation(const Quaternion& q)
{ 
    Orientation = q; Orientation.Normalize(); 
}
void RigidBody::SetAngularVelocity(const DirectX::XMVECTOR& av)
{ 
    AngularVelocity = av; 
}

void RigidBody::SetMass(float mass)
{
    InverseMass = (mass > 0.0f) ? 1.0f / mass : 0.0f; 
}

void RigidBody::SetInverseMass(float invMass)
{
    InverseMass = invMass; 
}

void RigidBody::SetLinearDamping(float d)
{
    m_LinearDamping = d; 
}

void RigidBody::SetAngularDamping(float d)
{
    AngularDamping = d; 
}

void RigidBody::SetInverseInertiaTensor(const  DirectX::XMMATRIX& tensor)
{
    InverseInertiaTensor = tensor; 
}

// Getters
DirectX::XMVECTOR RigidBody::GetPosition() const
{ 
    return Position; 
}

DirectX::XMVECTOR RigidBody::GetVelocity() const
{ 
    return Velocity;
}

DirectX::XMVECTOR RigidBody::GetAcceleration() const
{ 
    return Acceleration; 
}

DirectX::XMVECTOR RigidBody::GetAngularVelocity() const
{
    return AngularVelocity; 
}

Quaternion RigidBody::GetOrientation() const
{
    return Orientation; 
}

float RigidBody::GetMass() const
{
    return (InverseMass > 0.0f) ? 1.0f / InverseMass : INFINITY; 
}

float RigidBody::GetInverseMass() const
{
    return InverseMass;
}

DirectX::XMMATRIX RigidBody::GetInverseInertiaTensor() const
{
    return InverseInertiaTensor; 
}

bool RigidBody::HasFiniteMass() const
{ 
    return InverseMass > 0.0f;
}

float RigidBody::GetDamping() const
{
    return m_LinearDamping;
}
