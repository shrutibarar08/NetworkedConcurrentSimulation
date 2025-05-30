#include "ModelCapsule.h"

#include <random>

#include "CapsuleCollider.h"
#include "GuiManager/Widgets/ModelCapsuleUI.h"

ModelCapsule::ModelCapsule(const MODEL_INIT_DESC* desc)
	: IModel(desc)
{
	SetWidget(std::make_unique<ModelCapsuleUI>(this));
	m_Collider = std::make_unique<CapsuleCollider>(&m_RigidBody);
	m_RigidBody.SetMass(10);

	m_Colors = 
	{
		{
			{ 96.f / 255.f, 130.f / 255.f, 133.f / 255.f, 1.f },  // Dusty Cyan
			{ 112.f / 255.f, 128.f / 255.f, 144.f / 255.f, 1.f }, // Slate Gray
			{ 119.f / 255.f, 136.f / 255.f, 153.f / 255.f, 1.f }, // Light Slate
			{ 70.f / 255.f, 130.f / 255.f, 180.f / 255.f, 1.f },  // Steel Blue
			{ 88.f / 255.f, 108.f / 255.f, 121.f / 255.f, 1.f },  // Charcoal Blue
			{ 100.f / 255.f, 149.f / 255.f, 237.f / 255.f, 1.f }, // Cornflower Blue
			{ 79.f / 255.f, 102.f / 255.f, 120.f / 255.f, 1.f },  // Dusty Blue
			{ 105.f / 255.f, 105.f / 255.f, 105.f / 255.f, 1.f }, // Dim Gray
		},
	};
}

float ModelCapsule::GetRadius() const
{
	if (!m_Collider) return 1.0f; // default
	return m_Collider->GetRadius();
}

float ModelCapsule::GetHeight() const
{
	if (!m_Collider) return 1.0f; // default
	return m_Collider->GetHeight();
}

void ModelCapsule::SetRadius(float radius)
{
	if (!m_Collider) return;
	m_Collider->SetRadius(radius);
}

void ModelCapsule::SetHeight(float height)
{
	if (!m_Collider) return;
	m_Collider->SetHeight(height);
}

ICollider* ModelCapsule::GetCollider() const
{
	if (m_Collider) return m_Collider.get();
	return nullptr;
}

std::vector<VERTEX> ModelCapsule::BuildVertex()
{
	std::vector<VERTEX> vertices;

	const float radius = GetRadius();
	const float height = GetHeight();
	const UINT rings = GetRings();
	const UINT segments = GetSegments();

	// === Defensive color selection ===
	std::random_device rd;
	std::mt19937 rng(rd());

	std::vector<DirectX::XMFLOAT4> fallbackColorSet = {
		{1.0f, 1.0f, 1.0f, 1.0f} // plain white
	};

	const auto& colorSets = m_Colors.empty() ? std::vector<std::vector<DirectX::XMFLOAT4>>{fallbackColorSet} : m_Colors;

	std::uniform_int_distribution<size_t> setDist(0, colorSets.size() - 1);
	const auto& colorSet = colorSets[setDist(rng)];

	const bool emptySet = colorSet.empty();
	std::uniform_int_distribution<size_t> colorDist(0, emptySet ? 0 : colorSet.size() - 1);

	auto getRandomColor = [&]()
		{
			if (emptySet)
				return DirectX::XMFLOAT4(1.f, 1.f, 1.f, 1.f);
			return colorSet[colorDist(rng)];
		};

	// === Cylinder Section ===
	for (UINT y = 0; y <= rings; ++y)
	{
		float t = static_cast<float>(y) / rings;
		float cy = -height * 0.5f + t * height;

		for (UINT x = 0; x <= segments; ++x)
		{
			float angle = static_cast<float>(x) / segments * DirectX::XM_2PI;
			float cx = cosf(angle) * radius;
			float cz = sinf(angle) * radius;

			const auto& color = getRandomColor();
			vertices.emplace_back(cx, cy, cz, color.x, color.y, color.z, color.w);
		}
	}

	// === Top Hemisphere ===
	for (UINT y = 0; y <= rings; ++y)
	{
		float phi = static_cast<float>(y) / rings * DirectX::XM_PIDIV2;
		float sinPhi = sinf(phi);
		float cosPhi = cosf(phi);

		for (UINT x = 0; x <= segments; ++x)
		{
			float theta = static_cast<float>(x) / segments * DirectX::XM_2PI;
			float sinTheta = sinf(theta);
			float cosTheta = cosf(theta);

			float px = radius * sinPhi * cosTheta;
			float py = radius * cosPhi + height * 0.5f;
			float pz = radius * sinPhi * sinTheta;

			const auto& color = getRandomColor();
			vertices.emplace_back(px, py, pz, color.x, color.y, color.z, color.w);
		}
	}

	// === Bottom Hemisphere ===
	for (UINT y = 0; y <= rings; ++y)
	{
		float phi = static_cast<float>(y) / rings * DirectX::XM_PIDIV2;
		float sinPhi = sinf(phi);
		float cosPhi = cosf(phi);

		for (UINT x = 0; x <= segments; ++x)
		{
			float theta = static_cast<float>(x) / segments * DirectX::XM_2PI;
			float sinTheta = sinf(theta);
			float cosTheta = cosf(theta);

			float px = radius * sinPhi * cosTheta;
			float py = -radius * cosPhi - height * 0.5f;
			float pz = radius * sinPhi * sinTheta;

			const auto& color = getRandomColor();
			vertices.emplace_back(px, py, pz, color.x, color.y, color.z, color.w);
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

void ModelCapsule::SaveChildSweetData(SweetLoader& sweetData)
{
	if (!GetCollider()) return;

	CapsuleCollider* capsule = GetCollider()->As<CapsuleCollider>();
	if (!capsule) return;

	sweetData.GetOrCreate("Radius") = std::to_string(capsule->GetRadius());
	sweetData.GetOrCreate("Height") = std::to_string(capsule->GetHeight());
}

void ModelCapsule::LoadChildSweetData(const SweetLoader& sweetData)
{
	const SweetLoader& radiusNode = sweetData["Radius"];
	const SweetLoader& heightNode = sweetData["Height"];

	float radius = radiusNode.IsValid() ? radiusNode.AsFloat() : 0.0f;
	float height = heightNode.IsValid() ? heightNode.AsFloat() : 0.0f;

	if (!GetCollider()) return;

	CapsuleCollider* capsule = GetCollider()->As<CapsuleCollider>();
	if (!capsule) return;

	if (radius > 0.0f)
	{
		capsule->SetRadius(radius);
		SetRadius(radius);
	}
	if (height > 0.0f)
	{
		capsule->SetHeight(height);
		SetHeight(height);
	}
}
