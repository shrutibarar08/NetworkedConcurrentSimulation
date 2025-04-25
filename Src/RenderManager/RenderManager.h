#pragma once

#include <d3d11.h>
#include <wrl/client.h>
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
    WindowsSystem* mWindowSystem;

    std::vector<Microsoft::WRL::ComPtr<IDXGIAdapter>> mAdapters;
    int mSelectedAdapterIndex{ -1 };
    UINT mRefreshRateNumerator{ 60 };   // Default HZ
    UINT mRefreshRateDenominator{ 1 };

    std::vector<UINT> mSupportedMSAA;
	UINT mCurrentMSAA{ 8 };
    UINT mMSAACount{ 1 };
    UINT mMSAAQuality{ 0 };

    bool mQuit{ false };

    Microsoft::WRL::ComPtr<ID3D11Device> mDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> mDeviceContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> mRenderBuffer;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mRenderTargetView;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> mDepthBuffer;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> mDepthStencilState;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mDepthStencilView;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> mRasterizerState;
};
