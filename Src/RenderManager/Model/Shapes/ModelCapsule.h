#pragma once
#include "CapsuleCollider.h"
#include "RenderManager/Model/IModel.h"


class ModelCapsule final: public IModel
{
public:
	ModelCapsule(const MODEL_INIT_DESC* desc);
	~ModelCapsule() override = default;

	// Getters
	float GetRadius() const;
	float GetHeight() const;
	UINT GetRings() const { return m_Rings; }
	UINT GetSegments() const { return m_Segments; }

	// Setters
	void SetRadius(float radius);
	void SetHeight(float height);
	void SetRings(UINT rings) { m_Rings = rings; }
	void SetSegments(UINT segments) { m_Segments = segments; }
	ICollider* GetCollider() const override;

protected:
	std::vector<VERTEX> BuildVertex() override;
	std::vector<UINT> BuildIndex() override;

public:
	void SaveChildSweetData(SweetLoader& sweetData) override;
	void LoadChildSweetData(const SweetLoader& sweetData) override;

private:
	std::unique_ptr<CapsuleCollider> m_Collider{ nullptr };
	float m_Radius = 1.f;
	float m_Height = 1.f;
	UINT m_Rings = 8;
	UINT m_Segments = 16;
	std::vector<std::vector<DirectX::XMFLOAT4>> m_Colors;
};
