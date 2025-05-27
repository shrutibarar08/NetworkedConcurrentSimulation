#include "Render3DQueue.h"

#include <iostream>
#include <ranges>

#include "Utils/Logger.h"
#include "ICollider.h"


Render3DQueue::Render3DQueue(CameraController* controller, ID3D11Device* device)
{
	m_Device = device;
	m_CameraController = controller;
}

void Render3DQueue::AttachPhx(PhysicsManager* phx)
{
	m_PhysicsManager = phx;
}

bool Render3DQueue::AddModel(IModel* model)
{
	bool status = false;
	if (!m_ModelsToRender.contains(model->GetModelId()))
	{
		if (!model->IsBuilt()) model->Build(m_Device);
		m_ModelsToRender.emplace(model->GetModelId(), model);
		if (m_PhysicsManager)
		{
			m_PhysicsManager->AddRigidBody(model);

			if (m_ModelsToRender.size() == 2) m_2ndID = model->GetModelId();

		}
		status = true;
	}
	return status;
}

bool Render3DQueue::RemoveModel(const IModel* model)
{
	if (m_ModelsToRender.empty()) return false;

	bool status = false;
	if (m_ModelsToRender.contains(model->GetModelId()))
	{
		m_ModelsToRender.erase(model->GetModelId());
		m_PhysicsManager->RemoveRigidBody(model->GetModelId());
		status = true;
	}
	return status;
}

bool Render3DQueue::RemoveModel(uint64_t modelId)
{
	if (m_ModelsToRender.empty()) return false;

	bool status = false;

	if (m_ModelsToRender.contains(modelId))
	{
		m_ModelsToRender.erase(modelId);
		status = true;
	}
	return status;
}

bool Render3DQueue::UpdateVertexConstantBuffer(ID3D11DeviceContext* context)
{
	if (m_ModelsToRender.empty()) return false;

	MODEL_VERTEX_CB cb{};
	// Invert them
	cb.ViewMatrix = m_CameraController->GetViewMatrix();
	cb.ProjectionMatrix = m_CameraController->GetProjectionMatrix();

	// World & transformation remain identity
	cb.WorldMatrix = DirectX::XMMatrixIdentity();
	cb.Transformation = DirectX::XMMatrixIdentity();

	static int times = 0;
	for (auto& model : m_ModelsToRender | std::views::values)
	{
		if (!model->IsBuilt()) continue;

		cb.Transformation = model->GetCollider()->GetTransformationMatrix();
		model->UpdateVertexCB(context, &cb);
	}
	return true;
}

bool Render3DQueue::UpdatePixelConstantBuffer(ID3D11DeviceContext* context)
{
	if (m_ModelsToRender.empty()) return false;
	MODEL_PIXEL_CB cb{};
	cb.TotalTime = 0.0f; // TODO: Implement a global timer class

	std::vector<IModel*> models;

	models.reserve(m_ModelsToRender.size());
	for (auto& model: m_ModelsToRender | std::views::values) models.push_back(model);

	for (auto* model : models)
	{
		if (!model->IsBuilt()) continue;
		if (context) model->UpdatePixelCB(context, &cb);
	}
	return true;
}

void Render3DQueue::RenderAll(ID3D11DeviceContext* context)
{
	if (m_ModelsToRender.empty()) return;

	for (auto& model : m_ModelsToRender | std::views::values)
	{
		if (!model->IsBuilt()) continue;
		model->PresentModel(context);
	}
}
