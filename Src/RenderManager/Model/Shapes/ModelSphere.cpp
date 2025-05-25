#include "ModelSphere.h"

#include "SphereCollider.h"
#include "GuiManager/Widgets/ModelSphereUI.h"


ModelSphere::ModelSphere(const MODEL_INIT_DESC* desc)
	: IModel(desc)
{
	SetWidget(std::make_unique<ModelSphereUI>(this));
	m_Collider = std::make_unique<SphereCollider>(&m_RigidBody);
	m_RigidBody.SetMass(10);
}

std::vector<VERTEX> ModelSphere::BuildVertex()
{
    std::vector<VERTEX> vertices;

    for (UINT y = 0; y <= m_LatitudeSegments; ++y)
    {
        float theta = y * DirectX::XM_PI / m_LatitudeSegments;
        float sinTheta = sinf(theta);
        float cosTheta = cosf(theta);

        for (UINT x = 0; x <= m_LongitudeSegments; ++x)
        {
            float phi = x * DirectX::XM_2PI / m_LongitudeSegments;
            float sinPhi = sinf(phi);
            float cosPhi = cosf(phi);

            float px = cosPhi * sinTheta;
            float py = cosTheta;
            float pz = sinPhi * sinTheta;

            // Scale sphere to radius 0.5
            px *= 0.5f;
            py *= 0.5f;
            pz *= 0.5f;

            // Color gradient from polar to equator (Y axis based)
            float t = (float)y / m_LatitudeSegments;

            float r = 0.2f + 0.6f * sinf(t * DirectX::XM_PI); // Deep purple-pink
            float g = 0.8f * t;                               // Fades green
            float b = 1.0f - 0.5f * cosf(t * DirectX::XM_PI); // Subtle blue pulse

            vertices.emplace_back(px, py, pz, r, g, b, 1.0f);
        }
    }
    return vertices;
}

std::vector<UINT> ModelSphere::BuildIndex()
{
    std::vector<UINT> indices;

    for (UINT y = 0; y < m_LatitudeSegments; ++y)
    {
        for (UINT x = 0; x < m_LongitudeSegments; ++x)
        {
            UINT a = y * (m_LongitudeSegments + 1) + x;
            UINT b = a + m_LongitudeSegments + 1;

            indices.push_back(a);
            indices.push_back(b);
            indices.push_back(a + 1);

            indices.push_back(a + 1);
            indices.push_back(b);
            indices.push_back(b + 1);
        }
    }

    return indices;
}

void ModelSphere::SetLatitudeSegments(UINT segs)
{
	m_LatitudeSegments = segs;
}

void ModelSphere::SetLongitudeSegments(UINT segs)
{
	m_LongitudeSegments = segs;
}

UINT ModelSphere::GetLatitudeSegments() const
{
	return m_LatitudeSegments;
}

UINT ModelSphere::GetLongitudeSegments() const
{
	return m_LongitudeSegments;
}
