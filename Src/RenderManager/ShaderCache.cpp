#include "ShaderCache.h"
#include <d3dcompiler.h>
#include <filesystem>
#include <stdexcept>
#include <Windows.h>

ID3DBlob* ShaderCache::GetShader(const std::string& path)
{
    std::scoped_lock lock(m_Lock);

    auto it = m_Cache.find(path);
    if (it != m_Cache.end())
        return it->second.Get();

    auto blob = CompileOrLoad(path);
    m_Cache[path] = blob;
    return blob.Get();
}

Microsoft::WRL::ComPtr<ID3DBlob> ShaderCache::CompileOrLoad(const std::string& path)
{
    std::wstring wPath(path.begin(), path.end());
    Microsoft::WRL::ComPtr<ID3DBlob> blob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

    if (path.ends_with(".cso"))
    {
        HRESULT hr = D3DReadFileToBlob(wPath.c_str(), &blob);
        if (FAILED(hr))
            throw std::runtime_error("Failed to load compiled shader: " + path);
    }
    else if (path.ends_with(".hlsl"))
    {
        UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
        flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        std::string entryPoint = "main";
        std::string profile = path.find("PS") != std::string::npos ? "ps_4_0" : "vs_4_0";

        HRESULT hr = D3DCompileFromFile(
            wPath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
            entryPoint.c_str(), profile.c_str(),
            flags, 0, &blob, &errorBlob
        );

        if (FAILED(hr))
        {
            if (errorBlob)
                OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            throw std::runtime_error("Shader compilation failed: " + path);
        }
    }
    else
    {
        throw std::invalid_argument("Unsupported shader extension: " + path);
    }

    return blob;
}
