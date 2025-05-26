#pragma once

#include "CubeCollider.h"
#include "RenderManager/Model/IModel.h"

class ModelCube final: public IModel
{
public:
	ModelCube(const MODEL_INIT_DESC* desc);
	~ModelCube() override = default;
	ICollider* GetCollider() const override;

protected:
	std::vector<VERTEX> BuildVertex() override;
	std::vector<UINT> BuildIndex() override;

private:
	std::unique_ptr<CubeCollider> m_Collider{ nullptr };
};

