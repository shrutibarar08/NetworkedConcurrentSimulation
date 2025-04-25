#include "RenderManager.h"

#include <dxgi.h>
#include <dxgi1_2.h>
#include <iostream>

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
    //~ 
    if (!BuildParameter(sweetLoader)) return false;

	if (!QueryAdapter()) return false;
    if (!QueryMonitorRefreshRate()) return false;
    if (!BuildDeviceAndContext()) return false;

    QueryMSAA();
    if (!BuildSwapChain()) return false;

    if (!BuildRenderTargetView()) return false;
    if (!BuildDepthStencilView()) return false;
    if (!BuildRasterizationState()) return false;
    if (!BuildViewport()) return false;

    SetOMRenderAndDepth();

    return true;
}

void RenderManager::SetOMRenderAndDepth()
{
    mDeviceContext->OMSetRenderTargets(1,
        mRenderTargetView.GetAddressOf(),
        mDepthStencilView.Get());
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
        std::cerr << "Failed to create DXGI Factory.\n";
        return false;
    }

    UINT adapterIndex = 0;
    SIZE_T maxDedicatedVideoMemory = 0;

    mAdapters.clear();
    mSelectedAdapterIndex = -1;

    std::wcout << L"--- Enumerating GPU Adapters ---\n";
    while (true)
    {
        Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
        if (dxgiFactory->EnumAdapters(adapterIndex, adapter.GetAddressOf()) == DXGI_ERROR_NOT_FOUND)
            break;

        DXGI_ADAPTER_DESC desc;
        adapter->GetDesc(&desc);

        std::wcout << L"Adapter " << adapterIndex << L": " << desc.Description << std::endl;
        std::wcout << L"  VRAM: " << (desc.DedicatedVideoMemory / (1024 * 1024)) << L" MB\n";

        mAdapters.push_back(adapter);

        // Track the adapter with the most VRAM
        if (desc.DedicatedVideoMemory > maxDedicatedVideoMemory)
        {
            maxDedicatedVideoMemory = desc.DedicatedVideoMemory;
            mSelectedAdapterIndex = static_cast<int>(adapterIndex);
        }

        adapterIndex++;
    }

    if (mSelectedAdapterIndex == -1)
    {
        std::cerr << "No suitable adapter found.\n";
        return false;
    }

    DXGI_ADAPTER_DESC selectedDesc;
    mAdapters[mSelectedAdapterIndex]->GetDesc(&selectedDesc);

    std::wcout << L"\nSelected Adapter [" << mSelectedAdapterIndex << L"]: " << selectedDesc.Description << std::endl;
    std::wcout << L"  VRAM: " << (selectedDesc.DedicatedVideoMemory / (1024 * 1024)) << L" MB\n";

    return true;
}

bool RenderManager::QueryMonitorRefreshRate()
{
    Microsoft::WRL::ComPtr<IDXGIOutput> output;

    // Try primary output from selected adapter
    HRESULT hr = mAdapters[mSelectedAdapterIndex]->EnumOutputs(0, &output);

    if (FAILED(hr) || !output)
    {
        std::cerr << "Selected adapter has no outputs. Trying adapter 0 for monitor info...\n";

        // Fall back to adapter 0 (usually Intel GPU with monitor attached)
        if (mAdapters.size() > 0)
        {
            hr = mAdapters[0]->EnumOutputs(0, &output);
        }

        if (FAILED(hr) || !output)
        {
            std::cerr << "No monitor/output found on any adapter.\n";
            return false;
        }
    }

    DXGI_OUTPUT_DESC outputDesc;
    output->GetDesc(&outputDesc);

    std::wcout << L"Monitor: " << outputDesc.DeviceName << std::endl;

    DXGI_MODE_DESC desiredMode = {};
    desiredMode.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    DXGI_MODE_DESC closestMatch;
    hr = output->FindClosestMatchingMode(&desiredMode, &closestMatch, mDevice.Get());

    if (FAILED(hr))
    {
        std::cerr << "Failed to find closest matching mode.\n";
        return false;
    }

    UINT refreshRate = closestMatch.RefreshRate.Numerator / closestMatch.RefreshRate.Denominator;
    std::cout << "Monitor Refresh Rate: " << refreshRate << " Hz" << std::endl;

    mRefreshRateNumerator = closestMatch.RefreshRate.Numerator;
    mRefreshRateDenominator = closestMatch.RefreshRate.Denominator;

    return true;
}

bool RenderManager::BuildDeviceAndContext()
{
    if (mSelectedAdapterIndex < 0 || mSelectedAdapterIndex >= mAdapters.size())
    {
        std::cerr << "Invalid adapter index.\n";
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
        std::cerr << "Failed to create D3D11 device and context.\n";
        return false;
    }

    std::cout << "D3D11 Device created. Feature Level: 0x" << std::hex << selectedFeatureLevel << std::dec << "\n";
    return true;
}

bool RenderManager::QueryMSAA()
{
    if (!mDevice)
    {
        std::cerr << "Device not initialized. Cannot query MSAA.\n";
        return false;
    }

    mSupportedMSAA.clear();

    std::cout << "Querying supported MSAA sample counts...\n";

    for (UINT samples = 1; samples <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; ++samples)
    {
        UINT quality = 0;
        if (SUCCEEDED(mDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, samples, &quality)) && quality > 0)
        {
            mSupportedMSAA.push_back(samples);
            std::cout << "  " << samples << "x MSAA supported with quality levels: " << quality << "\n";
        }
    }

    if (mSupportedMSAA.empty())
    {
        std::cout << "No MSAA sample counts supported.\n";
        return false;
    }

    return true;
}

bool RenderManager::SetMSAA(UINT msaaValue)
{
    if (!mDevice)
    {
        std::cerr << "Device not initialized. Cannot set MSAA.\n";
        return false;
    }

    // Check if requested value is supported
    if (std::find(mSupportedMSAA.begin(), mSupportedMSAA.end(), msaaValue) == mSupportedMSAA.end())
    {
        std::cerr << "MSAA " << msaaValue << "x is not supported on this device.\n";
        return false;
    }

    UINT quality = 0;
    HRESULT hr = mDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, msaaValue, &quality);
    if (FAILED(hr) || quality == 0)
    {
        std::cerr << "Failed to retrieve MSAA quality level for " << msaaValue << "x.\n";
        return false;
    }

    mCurrentMSAA = msaaValue;
    mMSAACount = msaaValue;
    mMSAAQuality = quality - 1; // DirectX expects this to be [0, quality-1]

    std::cout << "MSAA set to " << mMSAACount << "x (quality level: " << mMSAAQuality << ")\n";
    return true;
}


bool RenderManager::BuildSwapChain()
{
    SetMSAA(8);

    if (!mDevice || mSelectedAdapterIndex < 0)
    {
        std::cerr << "Cannot build swap chain. Missing device or window handle.\n";
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGIFactory> dxgiFactory;
    mAdapters[mSelectedAdapterIndex]->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void**>(dxgiFactory.GetAddressOf()));

    RECT rt; GetClientRect(mWindowSystem->GetWindowHandle(), &rt);

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

    HRESULT hr = dxgiFactory->CreateSwapChain(
        mDevice.Get(),
        &scDesc,
        &mSwapChain
    );

    if (FAILED(hr))
    {
        std::cerr << "Failed to create SwapChain.\n";
        return false;
    }

    std::cout << "SwapChain created successfully.\n";
    return true;
}

bool RenderManager::BuildRenderTargetView()
{
    if (!mSwapChain)
    {
        std::cerr << "[RenderManager] SwapChain is null. Cannot build render target view.\n";
        return false;
    }

    HRESULT hr = mSwapChain->GetBuffer(0,
        __uuidof(ID3D11Texture2D),
        reinterpret_cast<void**>(mRenderBuffer.GetAddressOf()));
    if (FAILED(hr))
    {
        std::cerr << "[RenderManager] Failed to get back buffer from swap chain.\n";
        return false;
    }

    hr = mDevice->CreateRenderTargetView(mRenderBuffer.Get(),
        nullptr, &mRenderTargetView);
    if (FAILED(hr))
    {
        std::cerr << "[RenderManager] Failed to create render target view.\n";
        return false;
    }

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

    HRESULT hr = mDevice->CreateTexture2D(&depthDesc,
        nullptr, &mDepthBuffer);
    if (FAILED(hr))
    {
        std::cerr << "[RenderManager] Failed to create depth buffer texture.\n";
        return false;
    }

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

    depthStencilDesc.StencilEnable = true;
    depthStencilDesc.StencilReadMask = 0xFF;
    depthStencilDesc.StencilWriteMask = 0xFF;

    // Stencil operations if pixel is front-facing.
    depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Stencil operations if pixel is back-facing.
    depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    hr = mDevice->CreateDepthStencilState(&depthStencilDesc, &mDepthStencilState);
    if (FAILED(hr))
    {
        std::cout << "Failed To Create Depth Stencil State!\n";
        return false;
    }else
    {
        std::cout << "Created Depth Stencil State!\n";
    }

    // Set the depth stencil state.
    mDeviceContext->OMSetDepthStencilState(mDepthStencilState.Get(),
        1);

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = depthDesc.Format;
    dsvDesc.ViewDimension = (mMSAACount > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    hr = mDevice->CreateDepthStencilView(mDepthBuffer.Get(), &dsvDesc, &mDepthStencilView);
    if (FAILED(hr))
    {
        std::cerr << "[RenderManager] Failed to create depth stencil view.\n";
        return false;
    }

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
        std::cerr << "[RenderManager] Failed to create rasterizer state.\n";
        return false;
    }

    mDeviceContext->RSSetState(mRasterizerState.Get());
    return true;
}

bool RenderManager::BuildViewport()
{
    RECT rt;
    GetClientRect(mWindowSystem->GetWindowHandle(), &rt);

    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = static_cast<float>(rt.right - rt.left);
    viewport.Height = static_cast<float>(rt.bottom - rt.top);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    mDeviceContext->RSSetViewports(1, &viewport);
    return true;
}
