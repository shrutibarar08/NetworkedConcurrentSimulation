#pragma once

#include "RenderManager/Model/IModel.h"

class ModelCube final: public IModel
{
public:
	ModelCube(const MODEL_INIT_DESC* desc);
	~ModelCube() override = default;

protected:
	std::vector<VERTEX> BuildVertex() override;
	std::vector<UINT> BuildIndex() override;
};
