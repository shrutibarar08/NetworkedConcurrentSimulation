#pragma once
#include "ICollider.h"

class CapsuleCollider final : public ICollider
{
public:
	CapsuleCollider(RigidBody* body);
	bool CheckCollision(ICollider* other, Contact& outContact) override;
	ColliderType GetColliderType() const override;

	// Capsule-specific setters/getters
	void SetRadius(float radius);
	void SetHeight(float height);

	float GetRadius() const;
	float GetHeight() const;

	static void ClosestPtSegmentOBB(
		const DirectX::XMVECTOR& segStart,
		const DirectX::XMVECTOR& segEnd,
		const DirectX::XMVECTOR& boxCenter,
		const DirectX::XMVECTOR axes[3],
		const DirectX::XMVECTOR& halfExtents,
		DirectX::XMVECTOR& outCapsulePt,
		DirectX::XMVECTOR& outBoxPt
	);

	void SetScale(const DirectX::XMVECTOR& vector) override;
	DirectX::XMVECTOR GetScale() const override;

private:
	bool CheckCollisionWithCapsule(ICollider* other, Contact& outContact);
	bool CheckCollisionWithSphere(ICollider* other, Contact& outContact);
	bool CheckCollisionWithCube(ICollider* other, Contact& outContact);

	float ClosestPtSegmentSegment(
		const DirectX::XMVECTOR& p1, const DirectX::XMVECTOR& p2,
		const DirectX::XMVECTOR& q1, const DirectX::XMVECTOR& q2,
		DirectX::XMVECTOR& outPt1, DirectX::XMVECTOR& outPt2,
		float& outS, float& outT);

private:
	float m_Radius{ 1.0f };
	float m_Height{ 1.0f };
	DirectX::XMVECTOR m_Scale{ 1, 1, 1 };
};