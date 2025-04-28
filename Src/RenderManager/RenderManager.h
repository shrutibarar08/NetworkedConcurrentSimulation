#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "Camera/CameraController.h"
#include "SystemManager/Interface/ISystem.h"
#include "WindowManager/WindowsSystem.h"

class RenderManager : public ISystem
{
public:
    RenderManager(WindowsSystem* windowSystem);
    virtual ~RenderManager() override = default;

    RenderManager(const RenderManager&) = delete;
    RenderManager(RenderManager&&) = delete;
    RenderManager& operator=(const RenderManager&) = delete;
    RenderManager& operator=(RenderManager&&) = delete;

    // Main rendering loop or dispatch entry
	bool Run() override;
    bool ClearScene();
    bool SceneBegin();
    bool SceneEnd();

    // Build render config from SweetLoader
	bool Build(SweetLoader& sweetLoader) override;

    void SetOMRenderAndDepth();

private:
    bool BuildParameter(SweetLoader& sweetLoader);
    bool QueryAdapter();
    bool QueryMonitorRefreshRate();

    bool BuildDeviceAndContext();
    bool QueryMSAA();
    bool SetMSAA(UINT msaaValue);

    bool BuildSwapChain();
    bool BuildRenderTargetView();
    bool BuildDepthStencilView();
    bool BuildRasterizationState();
    bool BuildViewport() const;

private:
    WindowsSystem* m_WindowSystem;
    CameraManager m_CameraManager{};

    std::vector<Microsoft::WRL::ComPtr<IDXGIAdapter>> m_Adapters;
    int m_SelectedAdapterIndex{ -1 };
    UINT m_RefreshRateNumerator{ 60 };   // Default HZ
    UINT m_RefreshRateDenominator{ 1 };

    std::vector<UINT> m_SupportedMSAA;
	UINT m_CurrentMSAA{ 8 };
    UINT m_MSAACount{ 1 };
    UINT m_MSAAQuality{ 0 };

    Microsoft::WRL::ComPtr<ID3D11Device> m_Device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_DeviceContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_RenderBuffer;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_RenderTargetView;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_DepthBuffer;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_DepthStencilState;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_DepthStencilView;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_RasterizerState;
};
