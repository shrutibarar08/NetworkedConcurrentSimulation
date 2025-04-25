#include "RenderManager.h"

#include <dxgi.h>
#include <dxgi1_2.h>
#include <iostream>
#include <sstream>

#include "Utils/Logger.h"

RenderManager::RenderManager(WindowsSystem* windowSystem)
	: mWindowSystem(windowSystem)
{}

bool RenderManager::Run()
{
    ISystem::Run();

    while (!mQuit)
    {
        DWORD result = WaitForSingleObject(mGlobalEvent.GlobalEndEvent, 0);
        if (result == WAIT_OBJECT_0)
        {
            mQuit = true;
        }else
        {
            static float color[4]{ 0.25f, 0.50f, 0.75f, 1.0f };
            mDeviceContext->ClearRenderTargetView(
                mRenderTargetView.Get(),
                color);

            mDeviceContext->ClearDepthStencilView(
                mDepthStencilView.Get(),
                D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                1.0f,
                0.0f
            );

            mSwapChain->Present(0, 0);
        }
    }
    return true;
}

bool RenderManager::Build(SweetLoader& sweetLoader)
{
    LOG_INFO("RenderManager::Build() started.");

    if (!BuildParameter(sweetLoader))
    {
        LOG_FAIL("Failed to build parameters.");
        return false;
    }

    if (!QueryAdapter())
    {
        LOG_FAIL("Failed to query adapter.");
        return false;
    }

    if (!QueryMonitorRefreshRate())
    {
        LOG_FAIL("Failed to query monitor refresh rate.");
        return false;
    }

    if (!BuildDeviceAndContext())
    {
        LOG_FAIL("Failed to build device and context.");
        return false;
    }

    QueryMSAA();

    if (!BuildSwapChain())
    {
        LOG_FAIL("Failed to build swap chain.");
        return false;
    }

    if (!BuildRenderTargetView())
    {
        LOG_FAIL("Failed to build render target view.");
        return false;
    }

    if (!BuildDepthStencilView())
    {
        LOG_FAIL("Failed to build depth stencil view.");
        return false;
    }

    if (!BuildRasterizationState())
    {
        LOG_FAIL("Failed to build rasterizer state.");
        return false;
    }

    if (!BuildViewport())
    {
        LOG_FAIL("Failed to build viewport.");
        return false;
    }

    SetOMRenderAndDepth();

    LOG_SUCCESS("Build completed successfully.");
    return true;
}


void RenderManager::SetOMRenderAndDepth()
{
    mDeviceContext->OMSetRenderTargets(1,
        mRenderTargetView.GetAddressOf(),
        mDepthStencilView.Get());

    LOG_INFO("Bound render target and depth stencil view.");
}

bool RenderManager::BuildParameter(SweetLoader& sweetLoader)
{
    return true;
}

bool RenderManager::QueryAdapter()
{
    Microsoft::WRL::ComPtr<IDXGIFactory> dxgiFactory;
    HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(dxgiFactory.GetAddressOf()));

    if (FAILED(hr))
    {
        LOG_ERROR("Failed to create DXGI Factory.");
        return false;
    }

    UINT adapterIndex = 0;
    SIZE_T maxDedicatedVideoMemory = 0;

    mAdapters.clear();
    mSelectedAdapterIndex = -1;

    LOG_INFO("Enumerating GPU adapters...");

    while (true)
    {
        Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
        if (dxgiFactory->EnumAdapters(adapterIndex, adapter.GetAddressOf()) == DXGI_ERROR_NOT_FOUND)
            break;

        DXGI_ADAPTER_DESC desc;
        adapter->GetDesc(&desc);

        std::wostringstream adapterInfo;
        adapterInfo << L"Adapter " << adapterIndex << L": " << desc.Description;
        adapterInfo << L"\n  VRAM: " << (desc.DedicatedVideoMemory / (1024 * 1024)) << L" MB";
        OutputDebugStringW((adapterInfo.str() + L"\n").c_str()); // Optional for dev view

        mAdapters.push_back(adapter);

        if (desc.DedicatedVideoMemory > maxDedicatedVideoMemory)
        {
            maxDedicatedVideoMemory = desc.DedicatedVideoMemory;
            mSelectedAdapterIndex = static_cast<int>(adapterIndex);
        }

        adapterIndex++;
    }

    if (mSelectedAdapterIndex == -1)
    {
        LOG_FAIL("No suitable GPU adapter found.");
        return false;
    }

    DXGI_ADAPTER_DESC selectedDesc;
    mAdapters[mSelectedAdapterIndex]->GetDesc(&selectedDesc);

    std::ostringstream selectedInfo;
    selectedInfo << "Selected Adapter [" << mSelectedAdapterIndex << "]: ";

    std::wstring descWStr = selectedDesc.Description;
    selectedInfo << std::string(descWStr.begin(), descWStr.end()); // Convert to UTF-8-ish (rough)

    selectedInfo << " | VRAM: " << (selectedDesc.DedicatedVideoMemory / (1024 * 1024)) << " MB";

    LOG_SUCCESS(selectedInfo.str());

    return true;
}
bool RenderManager::QueryMonitorRefreshRate()
{
    Microsoft::WRL::ComPtr<IDXGIOutput> output;

    // Try to get output from selected adapter
    HRESULT hr = mAdapters[mSelectedAdapterIndex]->EnumOutputs(0, &output);

    if (FAILED(hr) || !output)
    {
        LOG_WARNING("Selected adapter has no monitor output. Falling back to adapter 0.");

        if (!mAdapters.empty())
        {
            hr = mAdapters[0]->EnumOutputs(0, &output);
        }

        if (FAILED(hr) || !output)
        {
            LOG_FAIL("No monitor/output found on any adapter.");
            return false;
        }
    }

    DXGI_OUTPUT_DESC outputDesc;
    output->GetDesc(&outputDesc);

    std::wstring wideName = outputDesc.DeviceName;
    std::string monitorName(wideName.begin(), wideName.end()); // Convert to narrow string
    LOG_INFO("Monitor: " + monitorName);

    DXGI_MODE_DESC desiredMode = {};
    desiredMode.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    DXGI_MODE_DESC closestMatch;
    hr = output->FindClosestMatchingMode(&desiredMode, &closestMatch, mDevice.Get());

    if (FAILED(hr))
    {
        LOG_ERROR("Failed to find closest matching display mode.");
        return false;
    }

    UINT refreshRate = closestMatch.RefreshRate.Numerator / closestMatch.RefreshRate.Denominator;
    mRefreshRateNumerator = closestMatch.RefreshRate.Numerator;
    mRefreshRateDenominator = closestMatch.RefreshRate.Denominator;

    LOG_SUCCESS("Monitor refresh rate: " + std::to_string(refreshRate) + " Hz");
    return true;
}

bool RenderManager::BuildDeviceAndContext()
{
    if (mSelectedAdapterIndex < 0 || mSelectedAdapterIndex >= mAdapters.size())
    {
        LOG_FAIL("Invalid adapter index for device creation.");
        return false;
    }

    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    D3D_FEATURE_LEVEL selectedFeatureLevel = {};

    HRESULT hr = D3D11CreateDevice(
        mAdapters[mSelectedAdapterIndex].Get(), // use selected adapter
        D3D_DRIVER_TYPE_UNKNOWN,
        nullptr,
        creationFlags,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &mDevice,
        &selectedFeatureLevel,
        &mDeviceContext
    );

    if (FAILED(hr))
    {
        LOG_ERROR("Failed to create D3D11 device and context.");
        return false;
    }

    std::ostringstream oss;
    oss << "D3D11 Device created. Feature Level: 0x" << std::hex << selectedFeatureLevel;
    LOG_SUCCESS(oss.str());

    return true;
}

bool RenderManager::QueryMSAA()
{
    if (!mDevice)
    {
        LOG_ERROR("Device not initialized. Cannot query MSAA.");
        return false;
    }

    mSupportedMSAA.clear();
    LOG_INFO("Querying supported MSAA sample counts...");

    for (UINT samples = 1; samples <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; ++samples)
    {
        UINT quality = 0;
        if (SUCCEEDED(mDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, samples, &quality)) && quality > 0)
        {
            mSupportedMSAA.push_back(samples);

            std::ostringstream oss;
            oss << "  " << samples << "x MSAA supported (Quality levels: " << quality << ")";
            LOG_INFO(oss.str());
        }
    }

    if (mSupportedMSAA.empty())
    {
        LOG_WARNING("No MSAA sample counts supported.");
        return false;
    }

    std::ostringstream oss;
    oss << "MSAA support query complete. " << mSupportedMSAA.size() << " levels detected.";
    LOG_SUCCESS(oss.str());

    return true;
}

bool RenderManager::SetMSAA(UINT msaaValue)
{
    if (!mDevice)
    {
        LOG_ERROR("Device not initialized. Cannot set MSAA.");
        return false;
    }

    // Check if requested value is supported
    if (std::find(mSupportedMSAA.begin(), mSupportedMSAA.end(), msaaValue) == mSupportedMSAA.end())
    {
        LOG_FAIL("MSAA " + std::to_string(msaaValue) + "x is not supported on this device.");
        return false;
    }

    UINT quality = 0;
    HRESULT hr = mDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, msaaValue, &quality);
    if (FAILED(hr) || quality == 0)
    {
        LOG_FAIL("Failed to retrieve MSAA quality level for " + std::to_string(msaaValue) + "x.");
        return false;
    }

    mCurrentMSAA = msaaValue;
    mMSAACount = msaaValue;
    mMSAAQuality = quality - 1; // DX expects [0..quality-1]

    std::ostringstream oss;
    oss << "MSAA set to " << mMSAACount << "x (quality level: " << mMSAAQuality << ")";
    LOG_SUCCESS(oss.str());

    return true;
}

bool RenderManager::BuildSwapChain()
{
    if (!SetMSAA(8))
    {
        LOG_WARNING("Requested MSAA 8x not supported. Falling back to previous/default setting.");
    }

    if (!mDevice || mSelectedAdapterIndex < 0 || !mWindowSystem)
    {
        LOG_FAIL("Cannot build swap chain. Missing device, adapter, or window handle.");
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGIFactory> dxgiFactory;
    HRESULT hr = mAdapters[mSelectedAdapterIndex]->GetParent(__uuidof(IDXGIFactory),
        reinterpret_cast<void**>(dxgiFactory.GetAddressOf()));

    if (FAILED(hr) || !dxgiFactory)
    {
        LOG_ERROR("Failed to retrieve DXGI factory from adapter.");
        return false;
    }

    RECT rt;
    GetClientRect(mWindowSystem->GetWindowHandle(), &rt);

    DXGI_SWAP_CHAIN_DESC scDesc = {};
    scDesc.BufferCount = 2;
    scDesc.BufferDesc.Width = rt.right - rt.left;
    scDesc.BufferDesc.Height = rt.bottom - rt.top;
    scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scDesc.BufferDesc.RefreshRate.Numerator = mRefreshRateNumerator;
    scDesc.BufferDesc.RefreshRate.Denominator = mRefreshRateDenominator;
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.OutputWindow = mWindowSystem->GetWindowHandle();
    scDesc.SampleDesc.Count = mMSAACount;
    scDesc.SampleDesc.Quality = mMSAAQuality;
    scDesc.Windowed = TRUE;
    scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    scDesc.Flags = 0;

    hr = dxgiFactory->CreateSwapChain(mDevice.Get(), &scDesc, &mSwapChain);
    if (FAILED(hr))
    {
        LOG_ERROR("Failed to create SwapChain.");
        return false;
    }

    std::ostringstream oss;
    oss << "SwapChain created successfully: "
        << scDesc.BufferDesc.Width << "x" << scDesc.BufferDesc.Height
        << " @ " << (scDesc.BufferDesc.RefreshRate.Numerator / scDesc.BufferDesc.RefreshRate.Denominator)
        << "Hz with " << mMSAACount << "x MSAA (Q" << mMSAAQuality << ")";

    LOG_SUCCESS(oss.str());
    return true;
}

bool RenderManager::BuildRenderTargetView()
{
    if (!mSwapChain)
    {
        LOG_FAIL("SwapChain is null. Cannot build render target view.");
        return false;
    }

    HRESULT hr = mSwapChain->GetBuffer(
        0,
        __uuidof(ID3D11Texture2D),
        reinterpret_cast<void**>(mRenderBuffer.GetAddressOf())
    );

    if (FAILED(hr))
    {
        LOG_ERROR("Failed to retrieve back buffer from swap chain.");
        return false;
    }

    hr = mDevice->CreateRenderTargetView(
        mRenderBuffer.Get(),
        nullptr,
        &mRenderTargetView
    );

    if (FAILED(hr))
    {
        LOG_ERROR("Failed to create render target view.");
        return false;
    }

    LOG_SUCCESS("Render target view created successfully.");
    return true;
}

bool RenderManager::BuildDepthStencilView()
{
    RECT rt;
    GetClientRect(mWindowSystem->GetWindowHandle(), &rt);
    UINT width = rt.right - rt.left;
    UINT height = rt.bottom - rt.top;

    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = mMSAACount;
    depthDesc.SampleDesc.Quality = mMSAAQuality;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    HRESULT hr = mDevice->CreateTexture2D(&depthDesc, nullptr, &mDepthBuffer);
    if (FAILED(hr))
    {
        LOG_ERROR("Failed to create depth buffer texture.");
        return false;
    }

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

    depthStencilDesc.StencilEnable = true;
    depthStencilDesc.StencilReadMask = 0xFF;
    depthStencilDesc.StencilWriteMask = 0xFF;

    // Front-facing stencil ops
    depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Back-facing stencil ops
    depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    hr = mDevice->CreateDepthStencilState(&depthStencilDesc, &mDepthStencilState);
    if (FAILED(hr))
    {
        LOG_ERROR("Failed to create depth stencil state.");
        return false;
    }

    mDeviceContext->OMSetDepthStencilState(mDepthStencilState.Get(), 1);

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = depthDesc.Format;
    dsvDesc.ViewDimension = (mMSAACount > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    hr = mDevice->CreateDepthStencilView(mDepthBuffer.Get(), &dsvDesc, &mDepthStencilView);
    if (FAILED(hr))
    {
        LOG_ERROR("Failed to create depth stencil view.");
        return false;
    }

    LOG_SUCCESS("Depth stencil buffer, state, and view created successfully.");
    return true;
}

bool RenderManager::BuildRasterizationState()
{
    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_BACK;
    rasterDesc.FrontCounterClockwise = FALSE;
    rasterDesc.DepthClipEnable = TRUE;

    HRESULT hr = mDevice->CreateRasterizerState(&rasterDesc, &mRasterizerState);
    if (FAILED(hr))
    {
        LOG_ERROR("Failed to create rasterizer state.");
        return false;
    }

    mDeviceContext->RSSetState(mRasterizerState.Get());

    LOG_SUCCESS("Rasterizer state created and bound to pipeline.");
    return true;
}

bool RenderManager::BuildViewport() const
{
    if (!mWindowSystem)
    {
        LOG_FAIL("Window system is not initialized. Cannot build viewport.");
        return false;
    }

    RECT rt;
    GetClientRect(mWindowSystem->GetWindowHandle(), &rt);
    UINT width = rt.right - rt.left;
    UINT height = rt.bottom - rt.top;

    if (width == 0 || height == 0)
    {
        LOG_FAIL("Invalid viewport dimensions (0x0).");
        return false;
    }

    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    mDeviceContext->RSSetViewports(1, &viewport);

    std::ostringstream oss;
    oss << "Viewport set to " << width << "x" << height;
    LOG_SUCCESS(oss.str());

    return true;
}
