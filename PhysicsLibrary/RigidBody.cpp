#include "pch.h"
#include "pch.h"
#include "RigidBody.h"
#include <cmath>
#include <iostream>
#include <string>

RigidBody::RigidBody()
    :
    InverseMass(1.0f),
    Orientation(1, 0, 0, 0)
{
    m_InverseInertiaTensorLocal = DirectX::XMMatrixIdentity();
    CalculateDerivedData();
}

void RigidBody::Integrate(float duration)
{
    CalculateDerivedData();

    // Atomically read inverse mass
    float invMass = InverseMass.load(std::memory_order_relaxed);
    if (invMass <= 0.0f) return;

    // === Linear motion ===
    DirectX::XMVECTOR pos = Position.Get();
    DirectX::XMVECTOR vel = Velocity.Get();
    DirectX::XMVECTOR acc = Acceleration.Get();
    DirectX::XMVECTOR force = ForceAccum.Get();

    pos = DirectX::XMVectorAdd(pos, DirectX::XMVectorScale(vel, duration));

    DirectX::XMVECTOR linearAcc = DirectX::XMVectorAdd(
        acc,
        DirectX::XMVectorScale(force, invMass)
    );

    vel = DirectX::XMVectorAdd(vel, DirectX::XMVectorScale(linearAcc, duration));

    float damping = m_LinearDamping.load(std::memory_order_relaxed);
    vel = DirectX::XMVectorScale(vel, std::pow(damping, duration));

    Position.Set(pos);
    Velocity.Set(vel);

    // === Angular motion ===
    DirectX::XMVECTOR angVel = AngularVelocity.Get();
    DirectX::XMVECTOR torque = TorqueAccum.Get();

    DirectX::XMVECTOR angularAcc = DirectX::XMVector3Transform(torque, InverseInertiaTensorWorld);
    angVel = DirectX::XMVectorAdd(angVel, DirectX::XMVectorScale(angularAcc, duration));

    float angDamp = AngularDamping.load(std::memory_order_relaxed);
    angVel = DirectX::XMVectorScale(angVel, std::pow(angDamp, duration));

    AngularVelocity.Set(angVel);

    // === Orientation ===
    float x = DirectX::XMVectorGetX(angVel);
    float y = DirectX::XMVectorGetY(angVel);
    float z = DirectX::XMVectorGetZ(angVel);

    Quaternion deltaRot(0.0f, x, y, z);
    deltaRot = deltaRot * Orientation;
    Orientation += deltaRot * 0.5f * duration;
    Orientation.Normalize();

    CalculateDerivedData();
    ClearAccumulators();
}

void RigidBody::AddForce(const DirectX::XMVECTOR& force)
{
    DirectX::XMVECTOR current = ForceAccum.Get();
    DirectX::XMVECTOR updated = DirectX::XMVectorAdd(current, force);
    ForceAccum.Set(updated);
}

void RigidBody::AddTorque(const DirectX::XMVECTOR& torque)
{
    DirectX::XMVECTOR current = TorqueAccum.Get();
    DirectX::XMVECTOR updated = DirectX::XMVectorAdd(current, torque);
    TorqueAccum.Set(updated);
}


void RigidBody::Integrate(float dt, IntegrationType type)
{
    CalculateDerivedData();

    float invMass = InverseMass.load(std::memory_order_relaxed);
    if (invMass <= 0.0f)
        return;

    DirectX::XMVECTOR acc = Acceleration.Get();
    DirectX::XMVECTOR force = ForceAccum.Get();

    DirectX::XMVECTOR acceleration = DirectX::XMVectorAdd(
        acc,
        DirectX::XMVectorScale(force, invMass)
    );

    DirectX::XMVECTOR vel = Velocity.Get();
    DirectX::XMVECTOR pos = Position.Get();
    DirectX::XMVECTOR lastPos = m_LastPosition.Get();

    switch (type)
    {
    case IntegrationType::Euler:
    {
        pos = DirectX::XMVectorAdd(pos, DirectX::XMVectorScale(vel, dt));
        vel = DirectX::XMVectorAdd(vel, DirectX::XMVectorScale(acceleration, dt));
        break;
    }
    case IntegrationType::SemiImplicitEuler:
    {
        vel = DirectX::XMVectorAdd(vel, DirectX::XMVectorScale(acceleration, dt));
        pos = DirectX::XMVectorAdd(pos, DirectX::XMVectorScale(vel, dt));
        break;
    }
    case IntegrationType::Verlet:
    {
        DirectX::XMVECTOR posDelta = DirectX::XMVectorSubtract(pos, lastPos);
        DirectX::XMVECTOR accelTerm = DirectX::XMVectorScale(acceleration, dt * dt);
        DirectX::XMVECTOR newPos = DirectX::XMVectorAdd(pos, DirectX::XMVectorAdd(posDelta, accelTerm));

        m_LastPosition.Set(pos);
        pos = newPos;
        break;
    }
    }

    float linearDamping = m_LinearDamping.load(std::memory_order_relaxed);
    vel = DirectX::XMVectorScale(vel, std::pow(linearDamping, dt));

    Position.Set(pos);
    Velocity.Set(vel);

    // === Angular motion ===
    DirectX::XMVECTOR torque = TorqueAccum.Get();
    DirectX::XMVECTOR angVel = AngularVelocity.Get();

    DirectX::XMVECTOR angularAcc = DirectX::XMVector3Transform(torque, m_InverseInertiaTensorLocal);
    angVel = DirectX::XMVectorAdd(angVel, DirectX::XMVectorScale(angularAcc, dt));

    float angularDamping = AngularDamping.load(std::memory_order_relaxed);
    angVel = DirectX::XMVectorScale(angVel, std::pow(angularDamping, dt));
    AngularVelocity.Set(angVel);

    // === Orientation ===
    Orientation.AddScaledVector(angVel, dt);
    Orientation.Normalize();

    // === Apply sleep threshold ===
    if (DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(vel)) < 1e-5f)
        Velocity.Set(DirectX::XMVectorZero());

    if (DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(angVel)) < 1e-5f)
        AngularVelocity.Set(DirectX::XMVectorZero());

    // === Done ===
    ClearAccumulators();
}


DirectX::XMMATRIX RigidBody::GetTransformMatrix()
{
    using namespace DirectX;
    XMMATRIX rotation = Orientation.ToRotationMatrix();
    XMMATRIX translation = XMMatrixTranslationFromVector(Position.Get());
    return rotation * translation;
}

void RigidBody::CalculateDerivedData()
{
    using namespace DirectX;

    Orientation.Normalize(); // Prevent drift

    XMMATRIX rotMatrix = XMMatrixRotationQuaternion(Orientation.ToXmVector());
    XMMATRIX translation = XMMatrixTranslationFromVector(Position.Get());

    TransformMatrix = rotMatrix * translation; // Full local-to-world matrix

    XMMATRIX rotTranspose = XMMatrixTranspose(rotMatrix);
    InverseInertiaTensorWorld = XMMatrixMultiply(
        XMMatrixMultiply(rotMatrix, m_InverseInertiaTensorLocal),
        rotTranspose
    );
}
void RigidBody::ClearAccumulators()
{
    ForceAccum.Set(DirectX::XMVectorZero());
    TorqueAccum.Set(DirectX::XMVectorZero());
}

// Setters
void RigidBody::SetPosition(const DirectX::XMVECTOR& pos)
{
    Position.Set(pos);
}

void RigidBody::SetVelocity(const DirectX::XMVECTOR& vel)
{
    Velocity.Set(vel);
}

void RigidBody::SetDamping(float d)
{
    m_LinearDamping.store(d, std::memory_order_relaxed);
}


void RigidBody::SetElasticity(float e)
{
    m_Elastic.store(e, std::memory_order_relaxed);
}


void RigidBody::SetRestitution(float v)
{
    m_Restitution.store(v, std::memory_order_relaxed);
}


void RigidBody::SetFriction(float v)
{
    m_Friction.store(v, std::memory_order_relaxed);
}


void RigidBody::SetAcceleration(const DirectX::XMVECTOR& acc)
{
    Acceleration.Set(acc);
}


void RigidBody::SetOrientation(const Quaternion& q)
{
    Orientation = q;
    Orientation.Normalize(); // direct math is fine
}


void RigidBody::SetAngularVelocity(const DirectX::XMVECTOR& av)
{
    AngularVelocity.Set(av);
}


void RigidBody::SetMass(float mass)
{
    InverseMass.store((mass > 0.0f) ? 1.0f / mass : 0.0f, std::memory_order_relaxed);
}


void RigidBody::SetInverseMass(float invMass)
{
    InverseMass.store(invMass, std::memory_order_relaxed);
}


void RigidBody::SetLinearDamping(float d)
{
    m_LinearDamping.store(d, std::memory_order_relaxed);
}


void RigidBody::SetAngularDamping(float d)
{
    AngularDamping.store(d, std::memory_order_relaxed);
}


void RigidBody::SetInverseInertiaTensor(const DirectX::XMMATRIX& tensor)
{
    m_InverseInertiaTensorLocal = tensor;
}


// Getters
DirectX::XMVECTOR RigidBody::GetPosition()
{
    return Position.Get();
}

DirectX::XMVECTOR RigidBody::GetVelocity()
{
    return Velocity.Get();
}

DirectX::XMVECTOR RigidBody::GetAcceleration()
{
    return Acceleration.Get();
}

DirectX::XMVECTOR RigidBody::GetAngularVelocity()
{
    return AngularVelocity.Get();
}

Quaternion RigidBody::GetOrientation() const
{
    auto result = Orientation;
    return result;
}

float RigidBody::GetMass() const
{
    float invMass = InverseMass.load(std::memory_order_relaxed);
    return (invMass > 0.0f) ? 1.0f / invMass : INFINITY;
}

float RigidBody::GetElasticity() const
{
    return m_Elastic.load(std::memory_order_relaxed);
}

float RigidBody::GetInverseMass() const
{
    return InverseMass.load(std::memory_order_relaxed);
}

DirectX::XMMATRIX RigidBody::GetInverseInertiaTensor() const
{
    return m_InverseInertiaTensorLocal;
}

DirectX::XMMATRIX RigidBody::GetInverseInertiaTensorWorld() const
{
    return InverseInertiaTensorWorld;
}

bool RigidBody::HasFiniteMass() const
{
    return InverseMass.load(std::memory_order_relaxed) > 0.0f;
}

float RigidBody::GetDamping() const
{
    return m_LinearDamping.load(std::memory_order_relaxed);
}

float RigidBody::GetAngularDamping()
{
    return AngularDamping.load(std::memory_order_relaxed);
}

float RigidBody::GetRestitution() const
{
    return m_Restitution.load(std::memory_order_relaxed);
}

float RigidBody::GetFriction() const
{
    return m_Friction.load(std::memory_order_relaxed);
}

void RigidBody::SetRestingState(bool state)
{
    m_Resting.store(state, std::memory_order_relaxed);
}

bool RigidBody::GetRestingState() const
{
    return m_Resting.load(std::memory_order_relaxed);
}

void RigidBody::ConstrainVelocity(const DirectX::XMVECTOR& contactNormal)
{
    const DirectX::XMVECTOR velocity = GetVelocity();
    const float dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(velocity, contactNormal));

    if (dot < 0.0f)
    {
        DirectX::XMVECTOR correction = DirectX::XMVectorScale(contactNormal, dot);
        DirectX::XMVECTOR constrained = DirectX::XMVectorSubtract(velocity, correction);
        SetVelocity(constrained);
    }
}

void RigidBody::ComputeInverseInertiaTensorBox(float width, float height, float depth)
{
    using namespace DirectX;

    float Ixx = (1.0f / 12.0f) * GetMass() * (height * height + depth * depth);
    float Iyy = (1.0f / 12.0f) * GetMass() * (width * width + depth * depth);
    float Izz = (1.0f / 12.0f) * GetMass() * (width * width + height * height);

    // Invert each component for the inverse tensor
    float invIxx = (Ixx > 0.0f) ? (1.0f / Ixx) : 0.0f;
    float invIyy = (Iyy > 0.0f) ? (1.0f / Iyy) : 0.0f;
    float invIzz = (Izz > 0.0f) ? (1.0f / Izz) : 0.0f;

    m_InverseInertiaTensorLocal = XMMatrixSet(
        invIxx, 0.0f, 0.0f, 0.0f,
        0.0f, invIyy, 0.0f, 0.0f,
        0.0f, 0.0f, invIzz, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

void RigidBody::ComputeInverseInertiaTensorSphere(float radius)
{
    float mass = GetMass();
    if (mass <= 0.0f)
    {
        m_InverseInertiaTensorLocal.r[0] = DirectX::XMVectorZero();
        m_InverseInertiaTensorLocal.r[1] = DirectX::XMVectorZero();
        m_InverseInertiaTensorLocal.r[2] = DirectX::XMVectorZero();
        m_InverseInertiaTensorLocal.r[3] = DirectX::XMVectorZero();
        return;
    }

    float factor = 2.0f / 5.0f * mass * radius * radius;
    float inv = 1.0f / factor;

    m_InverseInertiaTensorLocal = DirectX::XMMatrixIdentity();
    m_InverseInertiaTensorLocal.r[0] = DirectX::XMVectorSet(inv, 0, 0, 0);
    m_InverseInertiaTensorLocal.r[1] = DirectX::XMVectorSet(0, inv, 0, 0);
    m_InverseInertiaTensorLocal.r[2] = DirectX::XMVectorSet(0, 0, inv, 0);
    m_InverseInertiaTensorLocal.r[3] = DirectX::XMVectorSet(0, 0, 0, 1);
}

void RigidBody::ComputeInverseInertiaTensorCapsule(float radius, float height)
{
    using namespace DirectX;

    float mass = GetMass();
    if (mass <= 0.0f)
    {
        m_InverseInertiaTensorLocal.r[0] = DirectX::XMVectorZero();
        m_InverseInertiaTensorLocal.r[1] = DirectX::XMVectorZero();
        m_InverseInertiaTensorLocal.r[2] = DirectX::XMVectorZero();
        m_InverseInertiaTensorLocal.r[3] = DirectX::XMVectorZero();
        return;
    }

    float r2 = radius * radius;
    float h = height;
    float cylMass = mass * (height / (height + (4.0f / 3.0f) * radius)); // approx mass split
    float sphMass = mass - cylMass;

    // Cylinder inertia (Y-axis is height axis)
    float IxxCyl = 0.25f * cylMass * r2 + (1.0f / 12.0f) * cylMass * h * h;
    float IyyCyl = 0.5f * cylMass * r2;
    float IzzCyl = IxxCyl;

    // Sphere inertia
    float I_sphere = (2.0f / 5.0f) * sphMass * r2;

    // Use parallel axis theorem for spheres
    float offset = (h / 2.0f);
    float IxxSph = I_sphere + sphMass * offset * offset;
    float IyySph = I_sphere;
    float IzzSph = IxxSph;

    // Total inertia
    float Ixx = IxxCyl + IxxSph;
    float Iyy = IyyCyl + IyySph;
    float Izz = IzzCyl + IzzSph;

    // Invert for inverse tensor
    float invIxx = (Ixx > 0.0f) ? 1.0f / Ixx : 0.0f;
    float invIyy = (Iyy > 0.0f) ? 1.0f / Iyy : 0.0f;
    float invIzz = (Izz > 0.0f) ? 1.0f / Izz : 0.0f;

    m_InverseInertiaTensorLocal = XMMatrixSet(
        invIxx, 0.0f, 0.0f, 0.0f,
        0.0f, invIyy, 0.0f, 0.0f,
        0.0f, 0.0f, invIzz, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}
