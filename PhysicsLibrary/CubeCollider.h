#pragma once
#include "ICollider.h"


class CubeCollider final: public ICollider
{
public:
	CubeCollider(RigidBody* body);
	~CubeCollider() override = default;
	bool CheckCollision(ICollider* other, Contact& outContact) override;
	ColliderType GetColliderType() const override;
	RigidBody* GetRigidBody() const override;
	DirectX::XMVECTOR GetHalfExtents() const;

	void GetOBBAxes(const Quaternion& q, DirectX::XMVECTOR axes[3]);
	float ProjectOBB(const DirectX::XMVECTOR& axis, const DirectX::XMVECTOR axes[3], const DirectX::XMVECTOR& halfExtents);
	bool TryNormalize(DirectX::XMVECTOR& axis);

private:
	//~ Cube Collision Check
	bool CheckCollisionWithCube(ICollider* other, Contact& outContact);
	bool CheckCollisionWithSphere(ICollider* other, Contact& outContact);
	bool CheckCollisionWithCapsule(ICollider* other, Contact& outContact);

public:
	
	void SetScale(const DirectX::XMVECTOR& vector) override;
	DirectX::XMVECTOR GetScale() const override;

private:
	mutable SRWLOCK m_Lock{ SRWLOCK_INIT };
	DirectX::XMVECTOR m_Scale{ 1.0f, 1.0f, 1.0f };
};
