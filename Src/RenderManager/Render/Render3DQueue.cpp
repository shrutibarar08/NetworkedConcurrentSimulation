#include "Render3DQueue.h"

#include <ranges>

#include "Utils/Logger.h"


Render3DQueue::Render3DQueue(CameraController* controller, ID3D11Device* device)
{
	m_Device = device;
	m_CameraController = controller;
}

bool Render3DQueue::AddModel(IModel* model)
{
	bool status = false;
	AcquireSRWLockExclusive(&m_Lock);
	if (!m_ModelsToRender.contains(model->GetModelId()))
	{
		if (!model->IsBuilt()) model->Build(m_Device);
		m_ModelsToRender.emplace(model->GetModelId(), model);
		status = true;
	}
	ReleaseSRWLockExclusive(&m_Lock);
	return status;
}

bool Render3DQueue::RemoveModel(IModel* model)
{
	if (m_ModelsToRender.empty()) return false;

	bool status = false;
	AcquireSRWLockExclusive(&m_Lock);
	if (m_ModelsToRender.contains(model->GetModelId()))
	{
		m_ModelsToRender.erase(model->GetModelId());
		status = true;
	}
	ReleaseSRWLockExclusive(&m_Lock);
	return status;
}

bool Render3DQueue::RemoveModel(uint64_t modelId)
{
	if (m_ModelsToRender.empty()) return false;

	bool status = false;

	AcquireSRWLockExclusive(&m_Lock);
	if (m_ModelsToRender.contains(modelId))
	{
		m_ModelsToRender.erase(modelId);
		status = true;
	}
	ReleaseSRWLockExclusive(&m_Lock);
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

	std::vector<IModel*> models;

	AcquireSRWLockShared(&m_Lock);
	models.reserve(m_ModelsToRender.size());
	for (auto& model : m_ModelsToRender | std::views::values)
		models.push_back(model);
	ReleaseSRWLockShared(&m_Lock);

	for (auto* model : models)
		model->UpdateVertexCB(context, &cb);

	return true;
}

bool Render3DQueue::UpdatePixelConstantBuffer(ID3D11DeviceContext* context)
{
	if (m_ModelsToRender.empty()) return false;
	MODEL_PIXEL_CB cb{};
	cb.TotalTime = 0.0f; // TODO: Implement a global timer class

	std::vector<IModel*> models;

	AcquireSRWLockShared(&m_Lock);
	models.reserve(m_ModelsToRender.size());
	for (auto& model: m_ModelsToRender | std::views::values)
		models.push_back(model);
	ReleaseSRWLockShared(&m_Lock);

	for (auto* model : models)
		model->UpdatePixelCB(context, &cb);

	return true;
}

void Render3DQueue::RenderAll(ID3D11DeviceContext* context)
{
	if (m_ModelsToRender.empty()) return;

	for (auto& model : m_ModelsToRender | std::views::values)
	{
		AcquireSRWLockShared(&m_Lock);
		model->PresentModel(context);
		ReleaseSRWLockShared(&m_Lock);
	}
}
