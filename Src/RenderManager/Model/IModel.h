#pragma once
#include <atomic>

#include "Core/DefineDefault.h"

#include <d3d11.h>
#include <vector>
#include <string>
#include <wrl/client.h>


class IModel
{
public:
	IModel(const MODEL_INIT_DESC* desc);
	virtual ~IModel() = default;

	IModel(const IModel&) = delete;
	IModel(IModel&&) = delete;
	IModel& operator=(const IModel&) = delete;
	IModel& operator=(IModel&&) = delete;

	void Build(ID3D11Device* device);
	void PresentModel(ID3D11DeviceContext* context);

	void UpdateVertexCB(ID3D11DeviceContext* context, const MODEL_VERTEX_CB* cb);
	void UpdatePixelCB(ID3D11DeviceContext* context, const MODEL_PIXEL_CB* cb);

	uint64_t GetModelId() const;
	bool IsBuilt() const { return m_Built; }

protected:
	void BuildVertexBuffer(ID3D11Device* device);
	void BuildVertexShaderBlob(ID3D11Device* device);
	void BuildPixelShaderBlob(ID3D11Device* device);

	void BuildIndexBuffer(ID3D11Device* device);
	void BuildInputLayout(ID3D11Device* device);
	void BuildVertexConstantBuffer(ID3D11Device* device);
	void BuildPixelConstantBuffer(ID3D11Device* device);

	virtual std::vector<VERTEX> BuildVertex() = 0;
	virtual std::vector<UINT> BuildIndex() = 0;

protected:
	std::string m_VertexShaderPath;
	std::string m_PixelShaderPath;
	std::string m_ModelName;

	std::vector<VERTEX> m_Vertices;
	std::vector<UINT> m_Indices;

	UINT m_IndexCount;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_PixelConstantBuffer;
	Microsoft::WRL::ComPtr<ID3DBlob> m_VertexShaderBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> m_PixelShaderBlob;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;

	//~ Locks (safe threading)
	SRWLOCK m_Lock;
	inline static std::atomic_uint64_t s_ModelCounter = 0;
	uint64_t m_ModelID;
	bool m_Built{ false };
};
