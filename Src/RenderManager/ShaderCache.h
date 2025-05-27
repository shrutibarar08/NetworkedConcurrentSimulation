#pragma once
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <unordered_map>
#include <string>
#include <mutex>

class ShaderCache
{
public:
    static ID3DBlob* GetShader(const std::string& path);

private:
    static Microsoft::WRL::ComPtr<ID3DBlob> CompileOrLoad(const std::string& path);
    inline static std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>> m_Cache;
    inline static std::mutex m_Lock;
};
