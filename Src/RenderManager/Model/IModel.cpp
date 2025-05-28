#include "IModel.h"
#include "ExceptionManager/RenderException.h"
#include "Utils/Helper.h"

#include <cassert>
#include <d3dcompiler.h>

#include "RenderManager/ShaderCache.h"


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

void IModel::SetPayload(const CREATE_PAYLOAD& payload)
{
	// Set position
	m_RigidBody.SetPosition(DirectX::XMLoadFloat3(&payload.Position));
	
	m_RigidBody.SetOrientation(payload.Orientation);
	// Set linear velocity and acceleration
	m_RigidBody.SetVelocity(DirectX::XMLoadFloat3(&payload.Velocity));
	m_RigidBody.SetAcceleration(DirectX::XMLoadFloat3(&payload.Acceleration));

	// Set angular velocity
	m_RigidBody.SetAngularVelocity(DirectX::XMLoadFloat3(&payload.AngularVelocity));

	// Set scalar properties
	m_RigidBody.SetMass(payload.Mass);
	m_RigidBody.SetElasticity(payload.Elasticity);
	m_RigidBody.SetRestitution(payload.Restitution);
	m_RigidBody.SetFriction(payload.Friction);
	m_RigidBody.SetAngularDamping(payload.AngularDamping);
	m_RigidBody.SetLinearDamping(payload.LinearDamping);

	ColliderSate state = payload.Static ? ColliderSate::Static : ColliderSate::Dynamic;
	GetCollider()->SetColliderState(state);

	SetUiControlNeeded(payload.UiControlNeeded);
}

SweetLoader IModel::GetSweetData()
{
	SweetLoader sl{};

	// Position, Velocity, Acceleration, Angular Velocity
	DirectX::XMFLOAT3 pos, vel, acc, angVel;
	DirectX::XMStoreFloat3(&pos, m_RigidBody.GetPosition());
	DirectX::XMStoreFloat3(&vel, m_RigidBody.GetVelocity());
	DirectX::XMStoreFloat3(&acc, m_RigidBody.GetAcceleration());
	DirectX::XMStoreFloat3(&angVel, m_RigidBody.GetAngularVelocity());

	sl["Name"] = m_ModelName;
	sl["Position"]["x"] = std::to_string(pos.x);
	sl["Position"]["y"] = std::to_string(pos.y);
	sl["Position"]["z"] = std::to_string(pos.z);

	sl["Orientation"]["r"] = std::to_string(m_RigidBody.GetOrientation().GetR());
	sl["Orientation"]["i"] = std::to_string(m_RigidBody.GetOrientation().GetI());
	sl["Orientation"]["j"] = std::to_string(m_RigidBody.GetOrientation().GetJ());
	sl["Orientation"]["k"] = std::to_string(m_RigidBody.GetOrientation().GetK());

	sl["Velocity"]["x"] = std::to_string(vel.x);
	sl["Velocity"]["y"] = std::to_string(vel.y);
	sl["Velocity"]["z"] = std::to_string(vel.z);

	sl["Acceleration"]["x"] = std::to_string(acc.x);
	sl["Acceleration"]["y"] = std::to_string(acc.y);
	sl["Acceleration"]["z"] = std::to_string(acc.z);

	sl["AngularVelocity"]["x"] = std::to_string(angVel.x);
	sl["AngularVelocity"]["y"] = std::to_string(angVel.y);
	sl["AngularVelocity"]["z"] = std::to_string(angVel.z);

	// Scalars
	sl["Mass"] = std::to_string(m_RigidBody.GetMass());
	sl["Elasticity"] = std::to_string(m_RigidBody.GetElasticity());
	sl["InverseMass"] = std::to_string(m_RigidBody.GetInverseMass());
	sl["HasFiniteMass"] = m_RigidBody.HasFiniteMass() ? "true" : "false";
	sl["Damping"] = std::to_string(m_RigidBody.GetDamping());
	sl["AngularDamping"] = std::to_string(m_RigidBody.GetAngularDamping());
	sl["Restitution"] = std::to_string(m_RigidBody.GetRestitution());
	sl["Friction"] = std::to_string(m_RigidBody.GetFriction());
	sl["RestingState"] = m_RigidBody.GetRestingState() ? "true" : "false";

	// Collider Info
	sl["ColliderType"] = GetCollider()->GetColliderTypeName();
	sl["ColliderState"] = std::to_string(static_cast<int>(GetCollider()->GetColliderState()));

	DirectX::XMFLOAT3 scale{};
	DirectX::XMStoreFloat3(&scale, GetCollider()->GetScale());

	sl["Scale"]["x"] = std::to_string(scale.x);
	sl["Scale"]["y"] = std::to_string(scale.y);
	sl["Scale"]["z"] = std::to_string(scale.z);

	return sl;
}

void IModel::LoadFromSweetData(SweetLoader& sweetData)
{
	using namespace DirectX;

	auto loadVec3 = [&](SweetLoader& node) -> XMVECTOR
	{
		return XMVectorSet(
			node["x"].AsFloat(),
			node["y"].AsFloat(),
			node["z"].AsFloat(),
			0.0f
		);
	};

	m_RigidBody.SetPosition(loadVec3(sweetData["Position"]));
	m_RigidBody.SetVelocity(loadVec3(sweetData["Velocity"]));
	m_RigidBody.SetAcceleration(loadVec3(sweetData["Acceleration"]));
	m_RigidBody.SetAngularVelocity(loadVec3(sweetData["AngularVelocity"]));

	Quaternion q;
	q = Quaternion(
		sweetData["Orientation"]["r"].AsFloat(),
		sweetData["Orientation"]["i"].AsFloat(),
		sweetData["Orientation"]["j"].AsFloat(),
		sweetData["Orientation"]["k"].AsFloat()
	);
	m_RigidBody.SetOrientation(q);

	m_RigidBody.SetMass(sweetData["Mass"].AsFloat());
	m_RigidBody.SetElasticity(sweetData["Elasticity"].AsFloat());
	m_RigidBody.SetDamping(sweetData["Damping"].AsFloat());
	m_RigidBody.SetAngularDamping(sweetData["AngularDamping"].AsFloat());
	m_RigidBody.SetRestitution(sweetData["Restitution"].AsFloat());
	m_RigidBody.SetFriction(sweetData["Friction"].AsFloat());
	m_RigidBody.SetRestingState(sweetData["RestingState"].AsBool());

	// Collider data
	if (ICollider* collider = GetCollider())
	{
		XMVECTOR scaleVec = loadVec3(sweetData["Scale"]);
		collider->SetScale(scaleVec);

		int colliderState = sweetData["ColliderState"].AsInt();
		collider->SetColliderState(static_cast<ColliderSate>(colliderState));
	}
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
	ID3DBlob* blob = ShaderCache::GetShader(m_VertexShaderPath);
	if (!blob) THROW_EXCEPTION();

	HRESULT hr = device->CreateVertexShader(
		blob->GetBufferPointer(),
		blob->GetBufferSize(),
		nullptr,
		&m_VertexShader);
	
	THROW_RENDER_EXCEPTION_IF_FAILED(hr);

	LOG_INFO("Vertex Shader Built");
}

void IModel::BuildPixelShaderBlob(ID3D11Device* device)
{
	ID3DBlob* blob = ShaderCache::GetShader(m_PixelShaderPath);
	if (!blob) THROW_EXCEPTION();

	HRESULT hr = device->CreatePixelShader(
		blob->GetBufferPointer(),
		blob->GetBufferSize(),
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

	ID3DBlob* blob = ShaderCache::GetShader(m_VertexShaderPath);
	if (blob == nullptr) BuildVertexShaderBlob(device);

	HRESULT hr = device->CreateInputLayout(
		inputDesc,
		size,
		blob->GetBufferPointer(),
		blob->GetBufferSize(),
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
