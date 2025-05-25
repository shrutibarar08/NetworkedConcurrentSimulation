#include "ModelCapsule.h"

#include "GuiManager/Widgets/ModelCapsuleUI.h"

ModelCapsule::ModelCapsule(const MODEL_INIT_DESC* desc)
	: IModel(desc)
{
	SetWidget(std::make_unique<ModelCapsuleUI>(this));
	//m_Collider = std::make_unique<CapsuleCollider>(&m_RigidBody);
	m_RigidBody.SetMass(10);
}

std::vector<VERTEX> ModelCapsule::BuildVertex()
{
	std::vector<VERTEX> vertices;

	const float radius = GetRadius();
	const float height = GetHeight();
	const UINT rings = GetRings();
	const UINT segments = GetSegments();

	// === Cylinder Section ===
	for (UINT y = 0; y <= rings; ++y)
	{
		float t = (float)y / rings;
		float cy = -height * 0.5f + t * height;

		for (UINT x = 0; x <= segments; ++x)
		{
			float angle = (float)x / segments * DirectX::XM_2PI;
			float cx = cosf(angle) * radius;
			float cz = sinf(angle) * radius;

			float r = 0.5f + 0.5f * cosf(angle);
			float g = t;
			float b = 1.0f - t;

			vertices.emplace_back(cx, cy, cz, r, g, b, 1.0f);
		}
	}

	// === Top Hemisphere ===
	for (UINT y = 0; y <= rings; ++y)
	{
		float phi = (float)y / rings * DirectX::XM_PIDIV2;
		float sinPhi = sinf(phi);
		float cosPhi = cosf(phi);

		for (UINT x = 0; x <= segments; ++x)
		{
			float theta = (float)x / segments * DirectX::XM_2PI;
			float sinTheta = sinf(theta);
			float cosTheta = cosf(theta);

			float px = radius * sinPhi * cosTheta;
			float py = radius * cosPhi + height * 0.5f;
			float pz = radius * sinPhi * sinTheta;

			float r = sinf(phi);
			float g = 1.0f - r;
			float b = 0.5f + 0.5f * cosf(theta);

			vertices.emplace_back(px, py, pz, r, g, b, 1.0f);
		}
	}

	// === Bottom Hemisphere ===
	for (UINT y = 0; y <= rings; ++y)
	{
		float phi = (float)y / rings * DirectX::XM_PIDIV2;
		float sinPhi = sinf(phi);
		float cosPhi = cosf(phi);

		for (UINT x = 0; x <= segments; ++x)
		{
			float theta = (float)x / segments * DirectX::XM_2PI;
			float sinTheta = sinf(theta);
			float cosTheta = cosf(theta);

			float px = radius * sinPhi * cosTheta;
			float py = -radius * cosPhi - height * 0.5f;
			float pz = radius * sinPhi * sinTheta;

			float r = 0.2f + 0.8f * sinf(theta);
			float g = 0.5f * (1.0f - cosf(phi));
			float b = 1.0f - r;

			vertices.emplace_back(px, py, pz, r, g, b, 1.0f);
		}
	}

	return vertices;
}

std::vector<UINT> ModelCapsule::BuildIndex()
{
	std::vector<UINT> indices;

	const UINT rings = GetRings();
	const UINT segments = GetSegments();

	// === Cylinder ===
	for (UINT y = 0; y < rings; ++y)
	{
		for (UINT x = 0; x < segments; ++x)
		{
			UINT a = y * (segments + 1) + x;
			UINT b = a + segments + 1;

			indices.push_back(a);
			indices.push_back(b);
			indices.push_back(a + 1);

			indices.push_back(a + 1);
			indices.push_back(b);
			indices.push_back(b + 1);
		}
	}

	UINT vertexOffset = (rings + 1) * (segments + 1);

	// === Top Hemisphere ===
	for (UINT y = 0; y < rings; ++y)
	{
		for (UINT x = 0; x < segments; ++x)
		{
			UINT a = vertexOffset + y * (segments + 1) + x;
			UINT b = a + segments + 1;

			indices.push_back(a);
			indices.push_back(b);
			indices.push_back(a + 1);

			indices.push_back(a + 1);
			indices.push_back(b);
			indices.push_back(b + 1);
		}
	}

	vertexOffset += (rings + 1) * (segments + 1);

	// === Top Hemisphere ===
	for (UINT y = 0; y < rings; ++y)
	{
		for (UINT x = 0; x < segments; ++x)
		{
			UINT a = vertexOffset + y * (segments + 1) + x;
			UINT b = a + segments + 1;

			// FLIPPED winding order
			indices.push_back(a);
			indices.push_back(a + 1);
			indices.push_back(b);

			indices.push_back(a + 1);
			indices.push_back(b + 1);
			indices.push_back(b);
		}
	}

	return indices;
}
