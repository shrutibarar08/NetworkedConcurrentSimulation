#pragma once
#include <vector>

#include "ICollider.h"


class CubeCollider final : public ICollider
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

	void ComputeWorldAxes(DirectX::XMVECTOR outAxes[3]) const;
	void BuildSATTestAxes(const DirectX::XMVECTOR axesA[3],
		const DirectX::XMVECTOR axesB[3],
		std::vector<DirectX::XMVECTOR>& outAxes);
	bool TestOBBsWithSAT(
		const CubeCollider* boxA,
		const CubeCollider* boxB,
		const std::vector<DirectX::XMVECTOR>& axes,
		float& minPenetrationDepth,
		DirectX::XMVECTOR& outCollisionNormal
	);

	DirectX::XMVECTOR GetCenter() const;
	DirectX::XMVECTOR GetClosestPoint(DirectX::XMVECTOR point) const;


private:
	//~ Cube Collision Check
	bool CheckCollisionWithCube(ICollider* other, Contact& outContact);
	bool CheckCollisionWithSphere(ICollider* other, Contact& outContact);
	bool CheckCollisionWithCapsule(ICollider* other, Contact& outContact);

	Contact GenerateContactWithCube(CubeCollider* other);

public:

	void SetScale(const DirectX::XMVECTOR& vector) override;
	DirectX::XMVECTOR GetScale() const override;

private:
	DirectX::XMVECTOR m_Scale{ 1.0f, 1.0f, 1.0f };
};
