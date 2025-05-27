#pragma once
#include <atomic>

#include "Core/DefineDefault.h"
#include "ICollider.h"

#include <d3d11.h>
#include <memory>
#include <vector>
#include <string>
#include <wrl/client.h>

#include "GuiManager/Widgets/IWidget.h"

enum class SPAWN_OBJECT : uint8_t
{
	CUBE,
	SPHERE,
	CAPSULE
};

typedef struct CREATE_PAYLOAD
{
	SPAWN_OBJECT SpawnObject;

	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Velocity;
	DirectX::XMFLOAT3 Acceleration;
	DirectX::XMFLOAT3 AngularVelocity;

	float Mass;
	float Elasticity;
	float Restitution;
	float Friction;
	float AngularDamping;
	float LinearDamping;

	float SpawnTime = 0.0f; // time at which this object should be spawned

	bool operator<(const CREATE_PAYLOAD& other) const
	{
		return SpawnTime > other.SpawnTime;
	}

} CREATE_PAYLOAD;


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

	RigidBody* GetRigidBody();
	virtual ICollider* GetCollider() const = 0;

	void SetWidget(std::unique_ptr<IWidget> widget)
	{
		m_Widget = std::move(widget);
	}
	IWidget* GetWidget() const { return m_Widget.get(); }

	void SetName(const std::string& name)
	{
		m_Name = name;
	}
	std::string GetName() const { return m_Name; }

	void SetPayload(const CREATE_PAYLOAD& payload);

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
	std::string m_Name{ "NO NAME" };
	std::unique_ptr<IWidget> m_Widget{ nullptr };
	RigidBody m_RigidBody{};
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
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;

	//~ Locks (safe threading)
	SRWLOCK m_Lock;
	inline static std::atomic_uint64_t s_ModelCounter = 0;
	uint64_t m_ModelID;
	bool m_Built{ false };
};
