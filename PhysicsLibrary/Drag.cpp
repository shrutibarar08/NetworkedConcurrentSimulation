#include "pch.h"
#include "Drag.h"
#include "RigidBody.h"

#include <DirectXMath.h>

Drag::Drag(float k1, float k2)
    : K1(k1), K2(k2)
{}

void Drag::UpdateForce(RigidBody* body, float duration)
{
    DirectX::XMVECTOR velocity = body->GetVelocity();

    // Calculate the speed (magnitude of the velocity vector)
    float speed = DirectX::XMVectorGetX(DirectX::XMVector3Length(velocity));
    if (speed == 0.0f) return;

    // Calculate the drag coefficient
    float dragCoefficient = K1 * speed + K2 * speed * speed;

    DirectX::XMVECTOR dragDirection = DirectX::XMVector3Normalize(velocity);
    DirectX::XMVECTOR dragForce = DirectX::XMVectorScale(dragDirection, -dragCoefficient);

    body->AddForce(dragForce);
}
