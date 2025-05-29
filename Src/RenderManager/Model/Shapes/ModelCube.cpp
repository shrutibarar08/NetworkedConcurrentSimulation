#include "ModelCube.h"

#include <random>

#include "GuiManager/Widgets/ModelCubeUI.h"


ModelCube::ModelCube(const MODEL_INIT_DESC* desc)
    : IModel(desc)
{
    SetWidget(std::make_unique<ModelCubeUI>(this));
    m_Collider =  std::make_unique<CubeCollider>(&m_RigidBody);
    m_RigidBody.SetMass(10);

    m_Colors =
    {
        // Sky Blue Set
        {
            {  61.f / 255.f, 135.f / 255.f, 133.f / 255.f, 1.f },
            {  35.f / 255.f,  84.f / 255.f,  83.f / 255.f, 1.f },
            { 107.f / 255.f, 199.f / 255.f, 197.f / 255.f, 1.f },
            { 171.f / 255.f, 247.f / 255.f, 246.f / 255.f, 1.f },
            { 211.f / 255.f, 240.f / 255.f, 239.f / 255.f, 1.f },
            {  45.f / 255.f, 227.f / 255.f, 221.f / 255.f, 1.f },
            {   7.f / 255.f,  48.f / 255.f,  47.f / 255.f, 1.f },
            {   9.f / 255.f,  26.f / 255.f,  25.f / 255.f, 1.f },
        },

        // Red Set
        {
            { 139.f / 255.f,   0.f / 255.f,   0.f / 255.f, 1.f }, // Dark Red
            { 220.f / 255.f,  20.f / 255.f,  60.f / 255.f, 1.f }, // Crimson
            { 255.f / 255.f,  69.f / 255.f,   0.f / 255.f, 1.f }, // Red-Orange
            { 255.f / 255.f,  99.f / 255.f,  71.f / 255.f, 1.f }, // Tomato
            { 255.f / 255.f, 160.f / 255.f, 122.f / 255.f, 1.f }, // Light Salmon
            { 178.f / 255.f,  34.f / 255.f,  34.f / 255.f, 1.f }, // Firebrick
            { 255.f / 255.f, 105.f / 255.f, 180.f / 255.f, 1.f }, // Hot Pink
            { 255.f / 255.f, 182.f / 255.f, 193.f / 255.f, 1.f }, // Light Pink
        },

        // Yellow Set
        {
            { 255.f / 255.f, 255.f / 255.f,   0.f / 255.f, 1.f }, // Pure Yellow
            { 255.f / 255.f, 215.f / 255.f,   0.f / 255.f, 1.f }, // Gold
            { 255.f / 255.f, 239.f / 255.f, 184.f / 255.f, 1.f }, // Light Gold
            { 255.f / 255.f, 250.f / 255.f, 205.f / 255.f, 1.f }, // Lemon Chiffon
            { 238.f / 255.f, 232.f / 255.f, 170.f / 255.f, 1.f }, // Pale Goldenrod
            { 240.f / 255.f, 230.f / 255.f, 140.f / 255.f, 1.f }, // Khaki
            { 255.f / 255.f, 223.f / 255.f,   0.f / 255.f, 1.f }, // Amber
            { 218.f / 255.f, 165.f / 255.f,  32.f / 255.f, 1.f }, // Goldenrod
        }
    };
}

ICollider* ModelCube::GetCollider() const
{
    if (m_Collider) return m_Collider.get();
    return nullptr;
}

std::vector<VERTEX> ModelCube::BuildVertex()
{
    std::vector<DirectX::XMFLOAT3> positions =
    {
        // FRONT
        {-0.5f, -0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {0.5f, -0.5f, -0.5f},
        // BACK
        {0.5f, -0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}, {-0.5f, -0.5f, 0.5f},
        // LEFT
        {-0.5f, -0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, -0.5f}, {-0.5f, -0.5f, -0.5f},
        // RIGHT
        {0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, {0.5f, -0.5f, 0.5f},
        // TOP
        {-0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, -0.5f},
        // BOTTOM
        {-0.5f, -0.5f, 0.5f}, {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, 0.5f}
    };

    std::vector<VERTEX> vertices;
    vertices.reserve(positions.size());

    // === Defensive Check ===
    if (m_Colors.empty())
    {
        // Fallback: solid white cube
        for (const auto& pos : positions)
        {
            vertices.emplace_back(pos.x, pos.y, pos.z, 1.f, 1.f, 1.f, 1.f);
        }
        return vertices;
    }

    // Random setup
    std::random_device rd;
    std::mt19937 rng(rd());

    // Choose a random set
    std::uniform_int_distribution<size_t> setDist(0, m_Colors.size() - 1);
    const auto& colorSet = m_Colors[setDist(rng)];

    if (colorSet.empty())
    {
        // Fallback: solid gray if chosen set is empty
        for (const auto& pos : positions)
        {
            vertices.emplace_back(pos.x, pos.y, pos.z, 0.5f, 0.5f, 0.5f, 1.f);
        }
        return vertices;
    }

    // Pick random colors from within that set
    std::uniform_int_distribution<size_t> colorDist(0, colorSet.size() - 1);

    for (const auto& pos : positions)
    {
        const DirectX::XMFLOAT4& color = colorSet[colorDist(rng)];
        vertices.emplace_back(pos.x, pos.y, pos.z, color.x, color.y, color.z, color.w);
    }

    return vertices;
}

std::vector<UINT> ModelCube::BuildIndex()
{
    std::vector<UINT> indices;

    for (UINT face = 0; face < 6; ++face)
    {
        UINT base = face * 4;

        // Triangle 1: 0, 1, 2
        indices.push_back(base);
        indices.push_back(base + 1);
        indices.push_back(base + 2);

        // Triangle 2: 0, 2, 3
        indices.push_back(base);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }

    return indices;
}
