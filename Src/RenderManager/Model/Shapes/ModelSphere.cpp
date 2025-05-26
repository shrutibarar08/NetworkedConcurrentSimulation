#include "ModelSphere.h"

#include "SphereCollider.h"
#include "GuiManager/Widgets/ModelSphereUI.h"


ModelSphere::ModelSphere(const MODEL_INIT_DESC* desc)
	: IModel(desc)
{
	SetWidget(std::make_unique<ModelSphereUI>(this));
    m_Collider = std::make_unique<SphereCollider>(&m_RigidBody);
    m_Collider->SetRadius(GetRadius());
	m_RigidBody.SetMass(10);
}

std::vector<VERTEX> ModelSphere::BuildVertex()
{
    std::vector<VERTEX> vertices;

    float radius = GetRadius();
    UINT latSegments = GetLatitudeSegments();
    UINT lonSegments = GetLongitudeSegments();

    for (UINT lat = 0; lat <= latSegments; ++lat)
    {
        float theta = static_cast<float>(lat) * DirectX::XM_PI / static_cast<float>(latSegments); // [0, PI]
        float sinTheta = sinf(theta);
        float cosTheta = cosf(theta);

        for (UINT lon = 0; lon <= lonSegments; ++lon)
        {
            float phi = static_cast<float>(lon) * 2.0f * DirectX::XM_PI / static_cast<float>(lonSegments); // [0, 2PI]
            float sinPhi = sinf(phi);
            float cosPhi = cosf(phi);

            float x = radius * sinTheta * cosPhi;
            float y = radius * cosTheta;
            float z = radius * sinTheta * sinPhi;

            // Color based on position for fun
            float r = (x / radius + 1.0f) * 0.5f;
            float g = (y / radius + 1.0f) * 0.5f;
            float b = (z / radius + 1.0f) * 0.5f;

            vertices.emplace_back(x, y, z, r, g, b, 1.0f);
        }
    }
    return vertices;
}

std::vector<UINT> ModelSphere::BuildIndex()
{
    std::vector<UINT> indices;

    UINT latSegments = GetLatitudeSegments();
    UINT lonSegments = GetLongitudeSegments();

    for (UINT lat = 0; lat < latSegments; ++lat)
    {
        for (UINT lon = 0; lon < lonSegments; ++lon)
        {
            UINT current = lat * (lonSegments + 1) + lon;
            UINT next = current + lonSegments + 1;

            // Two triangles per quad on the sphere surface
            indices.push_back(current);
            indices.push_back(next);
            indices.push_back(current + 1);

            indices.push_back(current + 1);
            indices.push_back(next);
            indices.push_back(next + 1);
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

float ModelSphere::GetRadius() const
{
    return m_Radius;
}

void ModelSphere::SetRadius(float radius)
{
    m_Radius = radius;
}

ICollider* ModelSphere::GetCollider() const
{
    if (m_Collider) return m_Collider.get();
    return nullptr;
}
