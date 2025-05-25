#pragma once

#include <unordered_map>

#include "PhysicsManager/PhysicsManager.h"
#include "RenderManager/Camera/CameraController.h"
#include "RenderManager/Model/IModel.h"


class Render3DQueue
{
public:
	Render3DQueue(CameraController* controller, ID3D11Device* device);
	static void AttachPhx(PhysicsManager* phx);
	static bool AddModel(IModel* model);
	static bool RemoveModel(const IModel* model);
	static bool RemoveModel(uint64_t modelId);
	static bool UpdateVertexConstantBuffer(ID3D11DeviceContext* context);
	static bool UpdatePixelConstantBuffer(ID3D11DeviceContext* context);
	static void RenderAll(ID3D11DeviceContext* context);

private:
	inline static PhysicsManager* m_PhysicsManager = nullptr;
	inline static ID3D11Device* m_Device = nullptr;
	inline static CameraController* m_CameraController = nullptr;
	inline static std::unordered_map<uint64_t, IModel*> m_ModelsToRender = {};
	inline static uint64_t m_2ndID = 0;
};
