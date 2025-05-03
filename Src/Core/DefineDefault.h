#pragma once

#include <DirectXMath.h>
#include <string>
#include <windows.h>


namespace Barar
{
	namespace Windows
	{
		constexpr int DEFAULT_WIDTH{ 1280 };
		constexpr int DEFAULT_HEIGHT{ 720 };
		static std::wstring DEFAULT_WINDOW_NAME{ L"Concurrent Networked Simulation" };
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
	DirectX::XMMATRIX Transformation;
	DirectX::XMMATRIX ViewMatrix;
	DirectX::XMMATRIX ProjectionMatrix;
	DirectX::XMMATRIX WorldMatrix;
}MODEL_VERTEX_CB;


typedef struct alignas(16) MODEL_PIXEL_CB
{
	float TotalTime;
	float Pad[3];
}MODEL_PIXEL_CB;

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
