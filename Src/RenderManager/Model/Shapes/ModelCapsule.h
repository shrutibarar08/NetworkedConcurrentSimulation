#pragma once
#include "RenderManager/Model/IModel.h"


class ModelCapsule : public IModel
{
public:
	ModelCapsule(const MODEL_INIT_DESC* desc);
	~ModelCapsule() override = default;

	// Getters
	float GetRadius() const { return m_Radius; }
	float GetHeight() const { return m_Height; }
	UINT GetRings() const { return m_Rings; }
	UINT GetSegments() const { return m_Segments; }

	// Setters
	void SetRadius(float radius) { m_Radius = radius; }
	void SetHeight(float height) { m_Height = height; }
	void SetRings(UINT rings) { m_Rings = rings; }
	void SetSegments(UINT segments) { m_Segments = segments; }
	ICollider* GetCollider() const override { return nullptr; }


protected:
	std::vector<VERTEX> BuildVertex() override;
	std::vector<UINT> BuildIndex() override;

private:
	float m_Radius = 0.25f;
	float m_Height = 0.5f;
	UINT m_Rings = 8;
	UINT m_Segments = 16;
};
