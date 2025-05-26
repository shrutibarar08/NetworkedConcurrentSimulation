#include "ModelCube.h"
#include "GuiManager/Widgets/ModelCubeUI.h"


ModelCube::ModelCube(const MODEL_INIT_DESC* desc)
    : IModel(desc)
{
    SetWidget(std::make_unique<ModelCubeUI>(this));
    m_Collider =  std::make_unique<CubeCollider>(&m_RigidBody);
    m_RigidBody.SetMass(10);
}

ICollider* ModelCube::GetCollider() const
{
    if (m_Collider) return m_Collider.get();
    return nullptr;
}

std::vector<VERTEX> ModelCube::BuildVertex()
{
    std::vector<VERTEX> vertices =
    {
        // FRONT FACE (Z+)
        VERTEX(-0.5f, -0.5f, -0.5f, 1, 0, 0, 1),
        VERTEX(-0.5f,  0.5f, -0.5f, 1, 0, 0, 1),
        VERTEX(0.5f,  0.5f, -0.5f, 1, 0, 0, 1),
        VERTEX(0.5f, -0.5f, -0.5f, 1, 0, 0, 1),

        // BACK FACE (Z-)
        VERTEX(0.5f, -0.5f,  0.5f, 0, 1, 0, 1),
        VERTEX(0.5f,  0.5f,  0.5f, 0, 1, 0, 1),
        VERTEX(-0.5f,  0.5f,  0.5f, 0, 1, 0, 1),
        VERTEX(-0.5f, -0.5f,  0.5f, 0, 1, 0, 1),

        // LEFT FACE (X-)
        VERTEX(-0.5f, -0.5f,  0.5f, 0, 0, 1, 1),
        VERTEX(-0.5f,  0.5f,  0.5f, 0, 0, 1, 1),
        VERTEX(-0.5f,  0.5f, -0.5f, 0, 0, 1, 1),
        VERTEX(-0.5f, -0.5f, -0.5f, 0, 0, 1, 1),

        // RIGHT FACE (X+)
        VERTEX(0.5f, -0.5f, -0.5f, 1, 1, 0, 1),
        VERTEX(0.5f,  0.5f, -0.5f, 1, 1, 0, 1),
        VERTEX(0.5f,  0.5f,  0.5f, 1, 1, 0, 1),
        VERTEX(0.5f, -0.5f,  0.5f, 1, 1, 0, 1),

        // TOP FACE (Y+)
        VERTEX(-0.5f,  0.5f, -0.5f, 1, 0, 1, 1),
        VERTEX(-0.5f,  0.5f,  0.5f, 1, 0, 1, 1),
        VERTEX(0.5f,  0.5f,  0.5f, 1, 0, 1, 1),
        VERTEX(0.5f,  0.5f, -0.5f, 1, 0, 1, 1),

        // BOTTOM FACE (Y-)
        VERTEX(-0.5f, -0.5f,  0.5f, 0, 1, 1, 1),
        VERTEX(-0.5f, -0.5f, -0.5f, 0, 1, 1, 1),
        VERTEX(0.5f, -0.5f, -0.5f, 0, 1, 1, 1),
        VERTEX(0.5f, -0.5f,  0.5f, 0, 1, 1, 1),
    };

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
