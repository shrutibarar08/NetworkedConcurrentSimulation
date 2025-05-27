#include "RenderManager.h"

#include <dxgi.h>
#include <dxgi1_2.h>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "imgui_impl_dx11.h"
#include "ExceptionManager/RenderException.h"
#include "GuiManager/Widgets/RenderManagerUI.h"
#include "Utils/Logger.h"

RenderManager::RenderManager(WindowsSystem* windowSystem, PhysicsManager* phxManager)
	: m_WindowSystem(windowSystem), m_PhysicsManager(phxManager)
{
    m_DeviceMutex = CreateMutex
	(
        nullptr,
        FALSE,
        nullptr
    );

    InitializeSRWLock(&m_Lock);

    if (m_DeviceMutex == INVALID_HANDLE_VALUE)
    {
        throw std::runtime_error("failed to build device mutex!");
    }

    m_2dCamId = m_CameraManager.AddCamera("2DCam");
    m_3dCamId = m_CameraManager.AddCamera("3DCam");

    m_CameraManager.SetActiveCamera(m_3dCamId);
    m_CameraManager.GetActiveCamera()->SetAspectRatio(windowSystem->GetAspectRatio());

    SetWidget(std::make_unique<RenderManagerUI>(this));
}

bool RenderManager::Run()
{
    float m_TargetFrameTime = 1.0f / static_cast<float>(m_TargetGraphicsHz);
    if (m_Timer.HasElapsed(m_TargetFrameTime))
    {
        // === Measure actual elapsed time since last frame ===
        m_ActualFrameTime = m_Timer.Tick();
        m_ActualGraphicsHz = 1.0f / m_ActualFrameTime;

        ClearScene();
        SceneBegin();
        SceneEnd();
        m_Timer.Reset();
    }
    else
    {
        Sleep(1);
    }
    return true;
}

bool RenderManager::ClearScene()
{
    static float color[4]{ 0.25f, 0.20f, 0.75f, 1.0f };
    m_DeviceContext->ClearRenderTargetView(
        m_RenderTargetView.Get(),
        color);

    m_DeviceContext->ClearDepthStencilView(
        m_DepthStencilView.Get(),
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
        1.0f,
        0
    );

    return true;
}

bool RenderManager::SceneBegin()
{
    m_Render3DQueue->UpdatePixelConstantBuffer(m_DeviceContext.Get());
    m_Render3DQueue->UpdateVertexConstantBuffer(m_DeviceContext.Get());
    m_Render3DQueue->RenderAll(m_DeviceContext.Get());
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    return true;
}

bool RenderManager::SceneEnd()
{
    if (Draco::Renderer::VSYNC_ENABLED)
    {
        m_SwapChain->Present(1, 0);
    }else
    {
        m_SwapChain->Present(0, 0);
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

    if (!BuildDirectX())
    {
        LOG_FAIL("Failed to build DirectX Instance");
        return false;
    }

    auto* cam = m_CameraManager.GetActiveCamera();
    cam->AddTranslationY(1.0f);
    cam->AddTranslationZ(-10.0f);

    m_Render3DQueue = std::make_unique<Render3DQueue>(cam, m_Device.Get());
    m_Render3DQueue->AttachPhx(m_PhysicsManager);

    return true;
}

bool RenderManager::Shutdown()
{
    if (m_SwapChain)
    {
        m_SwapChain->SetFullscreenState(false, nullptr);
    }
	return ISystem::Shutdown();
}

bool RenderManager::BuildModel(IModel* model) const
{
    bool buildStatus = false;
    WaitForSingleObject(m_DeviceMutex, INFINITE);

    model->Build(m_Device.Get());
    buildStatus = true;

    ReleaseMutex(m_DeviceMutex);
    return buildStatus;
}

void RenderManager::SetOMRenderAndDepth()
{
    m_DeviceContext->OMSetRenderTargets(1,
        m_RenderTargetView.GetAddressOf(),
        m_DepthStencilView.Get());

    LOG_INFO("Bound render target and depth stencil view.");
}

void RenderManager::ResizeSwapChain(bool force)
{

    if (force)
    {
        LOG_WARNING("FORCE REBUILDING SWAP CHAIN!");
        BuildSwapChain();
        BuildRenderTargetView();
        BuildDepthStencilView();
        BuildViewport();
        BuildRasterizationState();
        SetOMRenderAndDepth();
        m_CameraManager.GetActiveCamera()->SetAspectRatio(m_WindowSystem->GetAspectRatio());
        return;
    }

    UINT width, height;
    RECT rc;
    GetClientRect(m_WindowSystem->GetWindowHandle(), &rc);
    width = rc.right - rc.left;
    height = rc.bottom - rc.top;

    if (m_PrevHeight == height && m_PrevWidth == width)
        return;

    LOG_INFO("Changing Viewport from: (" + std::to_string(m_PrevWidth) + ", " + std::to_string(m_PrevHeight) +
        ") -> (" + std::to_string(width) + ", " + std::to_string(height) + ")");

    m_PrevWidth = width;
    m_PrevHeight = height;

    m_RenderTargetView.Reset();
    m_DepthBuffer.Reset();
    m_RenderBuffer.Reset();
    m_DepthStencilView.Reset();
    m_DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
    m_DeviceContext->ClearState();

    HRESULT hr = m_SwapChain->ResizeBuffers(
        0, width, height,
        DXGI_FORMAT_UNKNOWN,
        DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
    );
    THROW_RENDER_EXCEPTION_IF_FAILED(hr);

    BuildRenderTargetView();
    BuildDepthStencilView();
    BuildViewport();
    BuildRasterizationState();
    SetOMRenderAndDepth();

    m_CameraManager.GetActiveCamera()->SetAspectRatio(m_WindowSystem->GetAspectRatio());
}

void RenderManager::ChangeMSAA(UINT msaa)
{
    if (msaa == m_CurrentMSAA) return;
    LOG_INFO("Updated MSAA Quality! from " + std::to_string(m_CurrentMSAA) + " to "
        + std::to_string(msaa));

    m_CurrentMSAA = msaa;
    BuildSwapChain();
    BuildRenderTargetView();
    BuildDepthStencilView();
    BuildViewport();
    BuildRasterizationState();
    SetOMRenderAndDepth();
    m_CameraManager.GetActiveCamera()->SetAspectRatio(m_WindowSystem->GetAspectRatio());
}

ID3D11DeviceContext* RenderManager::GetContext() const
{
    return m_DeviceContext.Get();
}

ID3D11Device* RenderManager::GetDevice() const
{
    return m_Device.Get();
}

DXGI_ADAPTER_DESC RenderManager::GetAdapterInformation()
{
    AcquireSRWLockShared(&m_Lock);
    DXGI_ADAPTER_DESC desc = m_CurrentAdapterDesc;
    ReleaseSRWLockShared(&m_Lock);
    return desc;
}

int RenderManager::GetRefreshRate() const
{
    return m_RefreshRateDenominator != 0
        ? m_RefreshRateNumerator / m_RefreshRateDenominator
        : 60;
}

int RenderManager::GetSelectedMSAA()
{
    AcquireSRWLockShared(&m_Lock);
    int val = m_CurrentMSAA;
    ReleaseSRWLockShared(&m_Lock);
    return m_CurrentMSAA;
}

std::vector<UINT> RenderManager::GetAllAvailableMSAA() const
{
    return m_SupportedMSAA;
}

CameraController* RenderManager::GetActiveCamera() const
{
    return m_CameraManager.GetActiveCamera();
}

void RenderManager::SetTargetGraphicsHz(int hz)
{
    m_TargetGraphicsHz = std::clamp(hz, 1, 1000);
}

int RenderManager::GetTargetGraphicsHz() const
{
    return m_TargetGraphicsHz;
}

float RenderManager::GetActualFrameTime() const
{
    return m_ActualFrameTime;
}

float RenderManager::GetActualGraphicsHz() const
{
    return m_ActualGraphicsHz;
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

    m_Adapters.clear();
    m_SelectedAdapterIndex = -1;

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

        m_Adapters.push_back(adapter);

        if (desc.DedicatedVideoMemory > maxDedicatedVideoMemory)
        {
            maxDedicatedVideoMemory = desc.DedicatedVideoMemory;
            m_SelectedAdapterIndex = static_cast<int>(adapterIndex);
        }

        adapterIndex++;
    }

    if (m_SelectedAdapterIndex == -1)
    {
        THROW_EXCEPTION();
    }

    m_Adapters[m_SelectedAdapterIndex]->GetDesc(&m_CurrentAdapterDesc);

    std::ostringstream selectedInfo;
    selectedInfo << "Selected Adapter [" << m_SelectedAdapterIndex << "]: ";

    std::wstring descWStr = m_CurrentAdapterDesc.Description;
    selectedInfo << std::string(descWStr.begin(), descWStr.end()); // Convert to UTF-8-ish (rough)

    selectedInfo << " | VRAM: " << (m_CurrentAdapterDesc.DedicatedVideoMemory / (1024 * 1024)) << " MB";

    LOG_SUCCESS(selectedInfo.str());

    return true;
}

bool RenderManager::BuildDirectX()
{
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

bool RenderManager::QueryMonitorRefreshRate()
{
    Microsoft::WRL::ComPtr<IDXGIOutput> output;

    // Try to get output from selected adapter
    HRESULT hr = m_Adapters[m_SelectedAdapterIndex]->EnumOutputs(0, &output);

    if (FAILED(hr) || !output)
    {
        DXGI_ADAPTER_DESC desc{};
        m_Adapters[0]->GetDesc(&desc);
        std::string name = std::string(
            std::begin(desc.Description),
            std::end(desc.Description));

        LOG_WARNING("Selected adapter has no monitor output. Falling back to: " + name);

        if (!m_Adapters.empty())
        {
            hr = m_Adapters[0]->EnumOutputs(0, &output);
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
    hr = output->FindClosestMatchingMode(&desiredMode, &closestMatch, m_Device.Get());

    if (FAILED(hr))
    {
        LOG_ERROR("Failed to find closest matching display mode.");
        return false;
    }

    UINT refreshRate = closestMatch.RefreshRate.Numerator / closestMatch.RefreshRate.Denominator;
    m_RefreshRateNumerator = closestMatch.RefreshRate.Numerator;
    m_RefreshRateDenominator = closestMatch.RefreshRate.Denominator;

    LOG_SUCCESS("Monitor refresh rate: " + std::to_string(refreshRate) + " Hz");
    return true;
}

bool RenderManager::BuildDeviceAndContext()
{
    if (m_SelectedAdapterIndex < 0 || m_SelectedAdapterIndex >= m_Adapters.size())
    {
        LOG_FAIL("Invalid adapter index for device creation.");
        return false;
    }

    UINT creationFlags = 0;
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
        m_Adapters[m_SelectedAdapterIndex].Get(), // use selected adapter
        D3D_DRIVER_TYPE_UNKNOWN,
        nullptr,
        creationFlags,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &m_Device,
        &selectedFeatureLevel,
        &m_DeviceContext
    );

    THROW_RENDER_EXCEPTION_IF_FAILED(hr);

    std::ostringstream oss;
    oss << "D3D11 Device created. Feature Level: 0x" << std::hex << selectedFeatureLevel;
    LOG_SUCCESS(oss.str());

    return true;
}

bool RenderManager::QueryMSAA()
{
    if (!m_Device)
    {
        LOG_ERROR("Device not initialized. Cannot query MSAA.");
        return false;
    }

    m_SupportedMSAA.clear();
    LOG_INFO("Querying supported MSAA sample counts...");

    for (UINT samples = 1; samples <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; ++samples)
    {
        UINT quality = 0;
        if (SUCCEEDED(m_Device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, samples, &quality)) && quality > 0)
        {
            m_SupportedMSAA.push_back(samples);

            std::ostringstream oss;
            oss << "  " << samples << "x MSAA supported (Quality levels: " << quality << ")";
            LOG_INFO(oss.str());
        }
    }

    if (m_SupportedMSAA.empty())
    {
        LOG_WARNING("No MSAA sample counts supported.");
        return false;
    }

    std::ostringstream oss;
    oss << "MSAA support query complete. " << m_SupportedMSAA.size() << " levels detected.";
    LOG_SUCCESS(oss.str());

    return true;
}

bool RenderManager::SetMSAA(UINT msaaValue)
{
    if (!m_Device)
    {
        LOG_ERROR("Device not initialized. Cannot set MSAA.");
        return false;
    }

    // Check if requested value is supported
    if (std::find(m_SupportedMSAA.begin(), m_SupportedMSAA.end(), msaaValue) == m_SupportedMSAA.end())
    {
        LOG_FAIL("MSAA " + std::to_string(msaaValue) + "x is not supported on this device.");
        return false;
    }

    UINT quality = 0;
    HRESULT hr = m_Device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, msaaValue, &quality);
    if (FAILED(hr) || quality == 0)
    {
        LOG_FAIL("Failed to retrieve MSAA quality level for " + std::to_string(msaaValue) + "x.");
        return false;
    }

    m_MSAACount = msaaValue;
    m_MSAAQuality = quality - 1; // DX expects [0..quality-1]

    std::ostringstream oss;
    oss << "MSAA set to " << m_MSAACount << "x (quality level: " << m_MSAAQuality << ")";
    LOG_SUCCESS(oss.str());

    return true;
}

bool RenderManager::BuildSwapChain()
{
    if (!SetMSAA(m_CurrentMSAA))
    {
        LOG_WARNING("Requested MSAA 8x not supported. Falling back to previous/default setting.");
    }

    if (!m_Device || m_SelectedAdapterIndex < 0 || !m_WindowSystem)
    {
        LOG_FAIL("Cannot build swap chain. Missing device, adapter, or window handle.");
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGIFactory> dxgiFactory;
    HRESULT hr = m_Adapters[m_SelectedAdapterIndex]->GetParent(__uuidof(IDXGIFactory),
        reinterpret_cast<void**>(dxgiFactory.GetAddressOf()));

    if (FAILED(hr) || !dxgiFactory)
    {
        THROW_RENDER_EXCEPTION_IF_FAILED(hr);
    }

    RECT rt;
    GetClientRect(m_WindowSystem->GetWindowHandle(), &rt);

    DXGI_SWAP_CHAIN_DESC scDesc = {};
    scDesc.BufferCount = 2;
    scDesc.BufferDesc.Width = rt.right - rt.left;
    scDesc.BufferDesc.Height = rt.bottom - rt.top;
    scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    if (Draco::Renderer::VSYNC_ENABLED)
    {
        scDesc.BufferDesc.RefreshRate.Numerator = m_RefreshRateNumerator;
        scDesc.BufferDesc.RefreshRate.Denominator = m_RefreshRateDenominator;
    }else
    {
        scDesc.BufferDesc.RefreshRate.Numerator = 0;
        scDesc.BufferDesc.RefreshRate.Denominator = 1;
    }
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.OutputWindow = m_WindowSystem->GetWindowHandle();
    scDesc.SampleDesc.Count = m_MSAACount;
    scDesc.SampleDesc.Quality = m_MSAAQuality;
    scDesc.Windowed = m_WindowSystem->IsFullScreen();
    scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    scDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    scDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    hr = dxgiFactory->CreateSwapChain(m_Device.Get(), &scDesc, &m_SwapChain);
    THROW_RENDER_EXCEPTION_IF_FAILED(hr);

    std::ostringstream oss;
    oss << "SwapChain created successfully: "
        << scDesc.BufferDesc.Width << "x" << scDesc.BufferDesc.Height
        << " @ " << (scDesc.BufferDesc.RefreshRate.Numerator / scDesc.BufferDesc.RefreshRate.Denominator)
        << "Hz with " << m_MSAACount << "x MSAA (Q" << m_MSAAQuality << ")";

    LOG_SUCCESS(oss.str());

    if (m_WindowSystem->IsFullScreen())
    {
        m_SwapChain->SetFullscreenState(TRUE, nullptr);
    }else
    {
        m_SwapChain->SetFullscreenState(FALSE, nullptr);
    }

    return true;
}

bool RenderManager::BuildRenderTargetView()
{
    if (!m_SwapChain)
    {
        THROW_EXCEPTION();
    }

    HRESULT hr = m_SwapChain->GetBuffer(
        0,
        __uuidof(ID3D11Texture2D),
        reinterpret_cast<void**>(m_RenderBuffer.GetAddressOf())
    );

    THROW_RENDER_EXCEPTION_IF_FAILED(hr);

    hr = m_Device->CreateRenderTargetView(
        m_RenderBuffer.Get(),
        nullptr,
        &m_RenderTargetView
    );

    THROW_RENDER_EXCEPTION_IF_FAILED(hr);

    LOG_SUCCESS("Render target view created successfully.");
    return true;
}

bool RenderManager::BuildDepthStencilView()
{
    RECT rt;
    GetClientRect(m_WindowSystem->GetWindowHandle(), &rt);
    UINT width = rt.right - rt.left;
    UINT height = rt.bottom - rt.top;

    LOG_INFO("Resized Depth View Width: " + std::to_string(width));
    LOG_INFO("Resized Depth View Height: " + std::to_string(height));

    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = m_MSAACount;
    depthDesc.SampleDesc.Quality = m_MSAAQuality;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    HRESULT hr = m_Device->CreateTexture2D(&depthDesc,
        nullptr, &m_DepthBuffer);
    THROW_RENDER_EXCEPTION_IF_FAILED(hr);

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

    hr = m_Device->CreateDepthStencilState(&depthStencilDesc, &m_DepthStencilState);
    THROW_RENDER_EXCEPTION_IF_FAILED(hr);

    m_DeviceContext->OMSetDepthStencilState(m_DepthStencilState.Get(), 1);

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = depthDesc.Format;
    dsvDesc.ViewDimension = (m_MSAACount > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    hr = m_Device->CreateDepthStencilView(m_DepthBuffer.Get(), &dsvDesc, &m_DepthStencilView);
    THROW_RENDER_EXCEPTION_IF_FAILED(hr);

    LOG_SUCCESS("Depth stencil buffer, state, and view created successfully.");
    return true;
}

bool RenderManager::BuildRasterizationState()
{
    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_NONE;
    rasterDesc.FrontCounterClockwise = FALSE;
    rasterDesc.DepthClipEnable = TRUE;

    HRESULT hr = m_Device->CreateRasterizerState(&rasterDesc, &m_RasterizerState);
    THROW_RENDER_EXCEPTION_IF_FAILED(hr);

    m_DeviceContext->RSSetState(m_RasterizerState.Get());

    LOG_SUCCESS("Rasterizer state created with CULL_NONE (both sides visible).");
    return true;
}

bool RenderManager::BuildViewport() const
{
    if (!m_WindowSystem)
    {
        THROW_EXCEPTION();
    }

    RECT rt;
    GetClientRect(m_WindowSystem->GetWindowHandle(), &rt);
    UINT width = rt.right - rt.left;
    UINT height = rt.bottom - rt.top;

    LOG_INFO("Resized View Port Width: " + std::to_string(width));
    LOG_INFO("Resized View Port Height: " + std::to_string(height));

    if (width == 0 || height == 0)
    {
        THROW_EXCEPTION();
    }

    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    m_DeviceContext->RSSetViewports(1, &viewport);

    std::ostringstream oss;
    oss << "Viewport set to " << width << "x" << height;
    LOG_SUCCESS(oss.str());

    return true;
}
