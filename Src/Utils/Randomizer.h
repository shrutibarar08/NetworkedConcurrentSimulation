#pragma once

#include <random>
#include <DirectXMath.h>


class Randomizer
{
public:
    Randomizer();
    // Float between [min, max]
    float Float(float min, float max);

    // Int between [min, max]
    int Int(int min, int max);
    // Bool with 50/50 chance
    bool Bool();
    // Random vector between two XMFLOAT3 ranges
    DirectX::XMFLOAT3 Vec3(const DirectX::XMFLOAT3& min, const DirectX::XMFLOAT3& max);

private:
    std::mt19937 m_Engine;

    // Helper to ensure min <= max
    template<typename T>
    void FixRange(T& a, T& b);
};

template <typename T>
void Randomizer::FixRange(T& a, T& b)
{
    if (a > b) std::swap(a, b);
}
