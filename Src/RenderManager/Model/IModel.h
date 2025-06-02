#pragma once
#include <atomic>

#include "Core/DefineDefault.h"
#include "ICollider.h"

#include <d3d11.h>
#include <memory>
#include <vector>
#include <string>
#include <wrl/client.h>

#include "FileManager/FileLoader/SweetLoader.h"
#include "GuiManager/Widgets/IWidget.h"
#include "Utils/LocalTimer.h"

enum class SPAWN_OBJECT : uint8_t
{
	CUBE,
	SPHERE,
	CAPSULE
};

typedef struct CREATE_PAYLOAD
{
	SPAWN_OBJECT SpawnObject{};

	DirectX::XMFLOAT3 Position{ 0.0f, 0.0f, 0.0f };
	Quaternion Orientation{};
	DirectX::XMFLOAT3 Velocity{ 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 Acceleration{ 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 AngularVelocity{ 0.0f, 0.0f, 0.0f };

	float Mass{ 10.0f };
	float Elasticity{ 0.5f };
	float Restitution{ 0.35f };
	float Friction{ 0.3f };
	float AngularDamping{ 0.4f };
	float LinearDamping{ 0.75f };

	float SpawnTime{ 0.01f };
	bool Static{ false };
	bool UiControlNeeded{ false };

	float Radius{ 1.0f };
	float Height{ 1.0f };
	float Width{ 1.0f };
	float Depth{ 1.0f };

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

	void UpdateVertexCB(ID3D11DeviceContext* context, MODEL_VERTEX_CB* cb);
	void UpdatePixelCB(ID3D11DeviceContext* context, MODEL_PIXEL_CB* cb);

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

	SweetLoader SaveSweetModelData();
	void LoadFromSweetData(const SweetLoader& sweetData);

	virtual void SaveChildSweetData(SweetLoader& sweetData) = 0;
	virtual void LoadChildSweetData(const SweetLoader& sweetData) = 0;

	bool IsUiControlNeeded() const { return m_UiControlNeeded; }
	void SetUiControlNeeded(bool flag) { m_UiControlNeeded = flag; }

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
	LocalTimer m_Timer{};
	bool m_UiControlNeeded{ true };
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
