#pragma once
#include "RenderManager/Model/IModel.h"


class ModelSphere: public IModel
{
public:
	ModelSphere(const MODEL_INIT_DESC* desc);
	~ModelSphere() override = default;

	void SetLatitudeSegments(UINT segs);
	void SetLongitudeSegments(UINT segs);

	UINT GetLatitudeSegments() const;
	UINT GetLongitudeSegments() const;

protected:
	std::vector<VERTEX> BuildVertex() override;
	std::vector<UINT> BuildIndex() override;

private:
	UINT m_LatitudeSegments = 16;
	UINT m_LongitudeSegments = 32;
};

