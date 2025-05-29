#pragma once
#include "SphereCollider.h"
#include "RenderManager/Model/IModel.h"


class ModelSphere final: public IModel
{
public:
	ModelSphere(const MODEL_INIT_DESC* desc);
	~ModelSphere() override = default;

	void SetLatitudeSegments(UINT segs);
	void SetLongitudeSegments(UINT segs);

	UINT GetLatitudeSegments() const;
	UINT GetLongitudeSegments() const;

	float GetRadius() const;
	void SetRadius(float radius);

	ICollider* GetCollider() const override;

protected:
	std::vector<VERTEX> BuildVertex() override;
	std::vector<UINT> BuildIndex() override;

private:
	std::unique_ptr<SphereCollider> m_Collider{ nullptr };
	float m_Radius{ 1.0f };
	UINT m_LatitudeSegments = 16;
	UINT m_LongitudeSegments = 32;
	std::vector<std::vector<DirectX::XMFLOAT4>> m_Colors{};
};

