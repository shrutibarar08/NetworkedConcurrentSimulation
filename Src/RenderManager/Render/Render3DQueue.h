#pragma once

#include <unordered_map>

#include "RenderManager/Camera/CameraController.h"
#include "RenderManager/Model/IModel.h"


class Render3DQueue
{
public:
	Render3DQueue(CameraController* controller, ID3D11Device* device);
	static bool AddModel(IModel* model);
	static bool RemoveModel(IModel* model);
	static bool RemoveModel(uint64_t modelId);
	static bool UpdateVertexConstantBuffer(ID3D11DeviceContext* context);
	static bool UpdatePixelConstantBuffer(ID3D11DeviceContext* context);
	static void RenderAll(ID3D11DeviceContext* context);

private:
	inline static ID3D11Device* m_Device = nullptr;
	inline static CameraController* m_CameraController = nullptr;
	inline static std::unordered_map<uint64_t, IModel*> m_ModelsToRender = {};
	inline static SRWLOCK m_Lock = SRWLOCK_INIT;
};
