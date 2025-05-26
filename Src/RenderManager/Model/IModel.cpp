#include "IModel.h"
#include "ExceptionManager/RenderException.h"
#include "Utils/Helper.h"

#include <cassert>
#include <d3dcompiler.h>


IModel::IModel(const MODEL_INIT_DESC* desc)
	: m_IndexCount(0), m_ModelID(++s_ModelCounter)
{
	m_VertexShaderPath = desc->VertexShaderPath;
	m_ModelName = desc->ModelName;
	m_PixelShaderPath = desc->PixelShaderPath;

	InitializeSRWLock(&m_Lock);
}

void IModel::Build(ID3D11Device* device)
{
	AcquireSRWLockExclusive(&m_Lock);
	LOG_WARNING("Attempting to build Model..");
	BuildVertexBuffer(device);
	BuildIndexBuffer(device);
	BuildPixelShaderBlob(device);
	BuildPixelConstantBuffer(device);
	BuildInputLayout(device);
	BuildVertexShaderBlob(device);
	BuildVertexConstantBuffer(device);
	LOG_SUCCESS("Model Built...");
	m_Built = true;
	ReleaseSRWLockExclusive(&m_Lock);
}

void IModel::PresentModel(ID3D11DeviceContext* context)
{

	assert(m_VertexBuffer && "Vertex buffer not set!");
	assert(m_IndexBuffer && "Index buffer not set!");
	assert(m_VertexConstantBuffer && "Constant buffer not set!");

	AcquireSRWLockShared(&m_Lock);

	context->IASetInputLayout(m_InputLayout.Get());

	UINT stride = sizeof(VERTEX);
	UINT offset = 0u;
	context->IASetVertexBuffers(0u,
		1u,
		m_VertexBuffer.GetAddressOf(), 
		&stride,
		&offset);

	context->IASetIndexBuffer(
		m_IndexBuffer.Get(),
		DXGI_FORMAT_R32_UINT,
		0u);

	context->VSSetConstantBuffers(0,
		1,
		m_VertexConstantBuffer.GetAddressOf());

	context->PSSetConstantBuffers(0u,
		1u,
		m_PixelConstantBuffer.GetAddressOf());

	context->VSSetShader(m_VertexShader.Get(), nullptr, 0u);
	context->PSSetShader(m_PixelShader.Get(), nullptr, 0u);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->DrawIndexed(m_IndexCount, 0u, 0u);

	ReleaseSRWLockShared(&m_Lock);
}

void IModel::UpdateVertexCB(ID3D11DeviceContext* context, const MODEL_VERTEX_CB* cb)
{
	if (!IsBuilt()) return;

	if (!cb || !context || !m_VertexConstantBuffer)
		throw std::invalid_argument("Invalid input to UpdateVertexCB.");

	AcquireSRWLockExclusive(&m_Lock);

	D3D11_MAPPED_SUBRESOURCE mappedResource = {};
	HRESULT hr = context->Map(
		m_VertexConstantBuffer.Get(),
		0,
		D3D11_MAP_WRITE_DISCARD,
		0,
		&mappedResource
	);
	THROW_RENDER_EXCEPTION_IF_FAILED(hr);

	memcpy(mappedResource.pData, cb, sizeof(MODEL_VERTEX_CB));

	context->Unmap(m_VertexConstantBuffer.Get(), 0);

	ReleaseSRWLockExclusive(&m_Lock);
}

void IModel::UpdatePixelCB(ID3D11DeviceContext* context, const MODEL_PIXEL_CB* cb)
{
	if (!IsBuilt()) return;
	if (!cb || !context || !m_PixelConstantBuffer)
		throw std::invalid_argument("Invalid input to UpdatePixelCB.");

	AcquireSRWLockExclusive(&m_Lock);

	D3D11_MAPPED_SUBRESOURCE mappedResource = {};

	HRESULT hr = context->Map(
		m_PixelConstantBuffer.Get(),
		0,
		D3D11_MAP_WRITE_DISCARD,
		0,
		&mappedResource
	);
	THROW_RENDER_EXCEPTION_IF_FAILED(hr);

	memcpy(mappedResource.pData, cb, sizeof(MODEL_PIXEL_CB));

	context->Unmap(m_PixelConstantBuffer.Get(), 0);

	ReleaseSRWLockExclusive(&m_Lock);
}


uint64_t IModel::GetModelId() const
{
	return m_ModelID;
}

RigidBody* IModel::GetRigidBody()
{
	return &m_RigidBody;
}

void IModel::BuildVertexBuffer(ID3D11Device* device)
{
	std::vector<VERTEX> vertices = BuildVertex();

	D3D11_BUFFER_DESC desc{};
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.ByteWidth = sizeof(VERTEX) * vertices.size();
	desc.CPUAccessFlags = static_cast<UINT>(0);
	desc.MiscFlags = static_cast<UINT>(0);
	desc.StructureByteStride = sizeof(VERTEX);
	desc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA data{};
	data.pSysMem = vertices.data();

	HRESULT hr = device->CreateBuffer(
		&desc,
		&data,
		&m_VertexBuffer
	);
	THROW_RENDER_EXCEPTION_IF_FAILED(hr);

	LOG_INFO("VertexBuffer Built");
}

void IModel::BuildVertexShaderBlob(ID3D11Device* device)
{
	std::wstring wPath = std::wstring(m_VertexShaderPath.begin(), m_VertexShaderPath.end());

	if (Help_IsSrcExtensionOf(m_VertexShaderPath, "cso"))
	{
		
		HRESULT hr = D3DReadFileToBlob(wPath.c_str(),&m_VertexShaderBlob);
		THROW_RENDER_EXCEPTION_IF_FAILED(hr);

	}else if (Help_IsSrcExtensionOf(m_VertexShaderPath, "hlsl"))
	{
		UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
		compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

		HRESULT hr = D3DCompileFromFile(
			wPath.c_str(),
			nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main", "vs_4_0",
			compileFlags, 0,
			&m_VertexShaderBlob,
			&errorBlob);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			}
			THROW_RENDER_EXCEPTION_HR(hr);
		}
	}
	else
	{
		throw std::invalid_argument("Unsupported shader file extension.");
	}

	HRESULT hr = device->CreateVertexShader(
		m_VertexShaderBlob->GetBufferPointer(),
		m_VertexShaderBlob->GetBufferSize(),
		nullptr,
		&m_VertexShader);
	
	THROW_RENDER_EXCEPTION_IF_FAILED(hr);

	LOG_INFO("Vertex Shader Built");
}

void IModel::BuildPixelShaderBlob(ID3D11Device* device)
{
	std::wstring wPath = std::wstring(
		m_PixelShaderPath.begin(),
		m_PixelShaderPath.end()
	);

	if (Help_IsSrcExtensionOf(m_PixelShaderPath, "cso"))
	{

		HRESULT hr = D3DReadFileToBlob
		(
			wPath.c_str(),
			&m_PixelShaderBlob
		);
		
		THROW_RENDER_EXCEPTION_IF_FAILED(hr);

	}
	else if (Help_IsSrcExtensionOf(m_PixelShaderPath, "hlsl"))
	{
		UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
		compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

		HRESULT hr = D3DCompileFromFile(
			wPath.c_str(),
			nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main", "ps_4_0",
			compileFlags, 0,
			&m_PixelShaderBlob,
			&errorBlob);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			}
			THROW_RENDER_EXCEPTION_HR(hr);
		}
	}
	else
	{
		throw std::invalid_argument("Unsupported shader file extension.");
	}


	HRESULT hr = device->CreatePixelShader(
		m_PixelShaderBlob->GetBufferPointer(),
		m_PixelShaderBlob->GetBufferSize(),
		nullptr,
		&m_PixelShader);

	THROW_RENDER_EXCEPTION_IF_FAILED(hr);


	LOG_INFO("Pixel Shader Built");
}

void IModel::BuildIndexBuffer(ID3D11Device* device)
{
	auto indices = BuildIndex();

	m_IndexCount = static_cast<UINT>(indices.size());
	LOG_INFO("Index Count: " + std::to_string(m_IndexCount));

	D3D11_BUFFER_DESC desc{};
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.ByteWidth = sizeof(UINT) * indices.size();
	desc.CPUAccessFlags = static_cast<UINT>(0);
	desc.MiscFlags = static_cast<UINT>(0);
	desc.StructureByteStride = sizeof(UINT);
	desc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA data{};
	data.pSysMem = indices.data();

	HRESULT hr = device->CreateBuffer(
		&desc,
		&data,
		&m_IndexBuffer
	);
	THROW_RENDER_EXCEPTION_IF_FAILED(hr);

	LOG_INFO("Index Buffer Built");
}

void IModel::BuildInputLayout(ID3D11Device* device)
{
	D3D11_INPUT_ELEMENT_DESC inputDesc[]
	{
		{
			"POSITION", 0u, DXGI_FORMAT_R32G32B32_FLOAT,
			0u, 0u, D3D11_INPUT_PER_VERTEX_DATA,
			0u
		},
	 {
			"COLOR", 0u, DXGI_FORMAT_R32G32B32A32_FLOAT,
			0u, 12u, D3D11_INPUT_PER_VERTEX_DATA,
			0u
		}
	};

	UINT size = ARRAYSIZE(inputDesc);

	if (m_VertexShaderBlob == nullptr) BuildVertexShaderBlob(device);


	HRESULT hr = device->CreateInputLayout(
		inputDesc,
		size,
		m_VertexShaderBlob->GetBufferPointer(),
		m_VertexShaderBlob->GetBufferSize(),
		&m_InputLayout
	);
	THROW_RENDER_EXCEPTION_IF_FAILED(hr);


	LOG_INFO("InputLayout Built");
}

void IModel::BuildVertexConstantBuffer(ID3D11Device* device)
{
	D3D11_BUFFER_DESC desc{};
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.ByteWidth = sizeof(MODEL_VERTEX_CB);
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0u;
	desc.StructureByteStride = 0u;
	desc.Usage = D3D11_USAGE_DYNAMIC;


	HRESULT hr = device->CreateBuffer(
		&desc,
		nullptr,
		&m_VertexConstantBuffer
	);

	THROW_RENDER_EXCEPTION_IF_FAILED(hr);

	LOG_INFO("Vertex Constant Buffer Built");
}

void IModel::BuildPixelConstantBuffer(ID3D11Device* device)
{
	D3D11_BUFFER_DESC desc{};
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.ByteWidth = sizeof(MODEL_PIXEL_CB);
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0u;
	desc.StructureByteStride = 0u;
	desc.Usage = D3D11_USAGE_DYNAMIC;


	HRESULT hr = device->CreateBuffer(
		&desc,
		nullptr,
		&m_PixelConstantBuffer
	);
	THROW_RENDER_EXCEPTION_IF_FAILED(hr);

	LOG_INFO("Pixel Constant Buffer Built");
}
