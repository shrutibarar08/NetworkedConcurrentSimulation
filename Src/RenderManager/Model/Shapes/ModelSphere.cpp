#include "ModelSphere.h"

#include <random>

#include "SphereCollider.h"
#include "GuiManager/Widgets/ModelSphereUI.h"


ModelSphere::ModelSphere(const MODEL_INIT_DESC* desc)
	: IModel(desc)
{
	SetWidget(std::make_unique<ModelSphereUI>(this));
    m_Collider = std::make_unique<SphereCollider>(&m_RigidBody);
    m_Collider->SetRadius(GetRadius());
	m_RigidBody.SetMass(10);

    m_Colors.push_back({
    { 123.f / 255.f,  63.f / 255.f,   0.f / 255.f, 1.f }, // Dark Chocolate
    { 92.f / 255.f,  51.f / 255.f,  23.f / 255.f, 1.f }, // Bitter Brown
    { 101.f / 255.f,  67.f / 255.f,  33.f / 255.f, 1.f }, // Saddle Chocolate
    { 139.f / 255.f,  69.f / 255.f,  19.f / 255.f, 1.f }, // Saddle Brown
    { 111.f / 255.f,  78.f / 255.f,  55.f / 255.f, 1.f }, // Cocoa
    { 120.f / 255.f,  66.f / 255.f,  18.f / 255.f, 1.f }, // Fudge
    { 160.f / 255.f,  82.f / 255.f,  45.f / 255.f, 1.f }, // Peru
    { 210.f / 255.f, 105.f / 255.f,  30.f / 255.f, 1.f }, // Chocolate
        });
}

std::vector<VERTEX> ModelSphere::BuildVertex()
{
    std::vector<VERTEX> vertices;

    float radius = GetRadius();
    UINT latSegments = GetLatitudeSegments();
    UINT lonSegments = GetLongitudeSegments();

    // === Defensive color set selection ===
    std::random_device rd;
    std::mt19937 rng(rd());

    std::vector<DirectX::XMFLOAT4> fallbackColorSet = {
        {1.0f, 1.0f, 1.0f, 1.0f} // fallback white
    };
    
    const auto& colorSets = m_Colors.empty() ? std::vector<std::vector<DirectX::XMFLOAT4>>{fallbackColorSet} : m_Colors;

    std::uniform_int_distribution<size_t> setDist(0, colorSets.size() - 1);
    const auto& colorSet = colorSets[setDist(rng)];

    const bool emptySet = colorSet.empty();
    std::uniform_int_distribution<size_t> colorDist(0, emptySet ? 0 : colorSet.size() - 1);

    auto getRandomColor = [&]() -> DirectX::XMFLOAT4
        {
            if (emptySet)
                return { 1.0f, 1.0f, 1.0f, 1.0f };
            return colorSet[colorDist(rng)];
        };

    for (UINT lat = 0; lat <= latSegments; ++lat)
    {
        float theta = static_cast<float>(lat) * DirectX::XM_PI / static_cast<float>(latSegments);
        float sinTheta = sinf(theta);
        float cosTheta = cosf(theta);

        for (UINT lon = 0; lon <= lonSegments; ++lon)
        {
            float phi = static_cast<float>(lon) * 2.0f * DirectX::XM_PI / static_cast<float>(lonSegments);
            float sinPhi = sinf(phi);
            float cosPhi = cosf(phi);

            float x = radius * sinTheta * cosPhi;
            float y = radius * cosTheta;
            float z = radius * sinTheta * sinPhi;

            const auto& color = getRandomColor();
            vertices.emplace_back(x, y, z, color.x, color.y, color.z, color.w);
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

void ModelSphere::SaveChildSweetData(SweetLoader& sweetData)
{
    if (!GetCollider()) return;

    SphereCollider* collider = GetCollider()->As<SphereCollider>();
    if (!collider) return;

    sweetData.GetOrCreate("Radius") = std::to_string(collider->GetRadius());
}

void ModelSphere::LoadChildSweetData(const SweetLoader& sweetData)
{
    if (!GetCollider()) return;

    SphereCollider* collider = GetCollider()->As<SphereCollider>();
    if (!collider) return;

    const SweetLoader& radiusNode = sweetData["Radius"];
    if (!radiusNode.IsValid()) return;

    float radius = radiusNode.AsFloat();
    if (radius > 0.0f)
    {
        collider->SetRadius(radius);
        SetRadius(radius);
    }
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
    if (!m_Collider) return 1.0f;
    return m_Collider->GetRadius();
}

void ModelSphere::SetRadius(float radius)
{
    if (!m_Collider) return;
    m_Collider->SetRadius(radius);
}

ICollider* ModelSphere::GetCollider() const
{
    if (m_Collider) return m_Collider.get();
    return nullptr;
}
