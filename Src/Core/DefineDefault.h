#pragma once

#include <DirectXMath.h>
#include <string>
#include <windows.h>


namespace Draco
{
	namespace Windows
	{
		constexpr int DEFAULT_WIDTH{ 1280 };
		constexpr int DEFAULT_HEIGHT{ 720 };
		static std::wstring DEFAULT_WINDOW_NAME{ L"Concurrent Networked Simulation" };

#ifdef _DEBUG
		constexpr bool FULL_SCREEN{ false };
#else
		constexpr bool FULL_SCREEN{ false };
#endif
	}

	namespace Renderer
	{
		constexpr bool VSYNC_ENABLED = true;
	}
}

typedef struct VERTEX
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT4 Color;

	VERTEX(float x, float y, float z, float r, float g, float b, float a)
		: Position(x, y, z), Color(r, g, b, a) {
	}
} VERTEX;

typedef struct MODEL_INIT_DESC
{
	std::string ModelName;
	std::string VertexShaderPath;
	std::string PixelShaderPath;
	std::string TexturePath;
}MODEL_INIT_DESC;

typedef struct alignas(16) MODEL_VERTEX_CB
{
	DirectX::XMMATRIX Transformation;    // 64 bytes
	DirectX::XMMATRIX ViewMatrix;        // 64 bytes
	DirectX::XMMATRIX ProjectionMatrix;  // 64 bytes
	DirectX::XMMATRIX WorldMatrix;       // 64 bytes

	float DeltaTime;                     // 4 bytes
	int IsStatic;                        // 4 bytes (bool padded to int for alignment)
	float Padding0[2];                   // 8 bytes to align next XMVECTOR

	DirectX::XMVECTOR AngularVelocity;   // 16 bytes
	DirectX::XMVECTOR Velocity;          // 16 bytes
} MODEL_VERTEX_CB;

typedef struct alignas(16) MODEL_PIXEL_CB
{
	float DeltaTime;                    // 4 bytes
	int IsStatic;                       // 4 bytes
	float Padding0[2];                  // 8 bytes to align next XMVECTOR

	DirectX::XMVECTOR AngularVelocity;  // 16 bytes
	DirectX::XMVECTOR Velocity;         // 16 bytes
} MODEL_PIXEL_CB;

typedef struct SYSTEM_EVENT_HANDLE
{
	HANDLE GlobalStartEvent;
	HANDLE GlobalEndEvent;
}SYSTEM_EVENT_HANDLE;

typedef struct LOGGER_INITIALIZE_DESC
{
	std::string FolderPath;
	std::string FilePath;
	bool EnableTerminal;
}LOGGER_INITIALIZE_DESC;

enum class MOUSE_BUTTON: uint8_t
{
	LEFT_MOUSE,
	MIDDLE_MOUSE,
	RIGHT_MOUSE
};