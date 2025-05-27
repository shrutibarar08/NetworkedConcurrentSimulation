#include "Randomizer.h"


Randomizer::Randomizer()
	: m_Engine(std::random_device{}())
{}

float Randomizer::Float(float min, float max)
{
    FixRange(min, max);
    if (min == max) return min;

    std::uniform_real_distribution<float> dist(min, max);
    return dist(m_Engine);
}

int Randomizer::Int(int min, int max)
{
    FixRange(min, max);
    if (min == max) return min;

    std::uniform_int_distribution<int> dist(min, max);
    return dist(m_Engine);
}

bool Randomizer::Bool()
{
    std::bernoulli_distribution dist(0.5);
    return dist(m_Engine);
}

DirectX::XMFLOAT3 Randomizer::Vec3(const DirectX::XMFLOAT3& min, const DirectX::XMFLOAT3& max)
{
    return {
           Float(min.x, max.x),
           Float(min.y, max.y),
           Float(min.z, max.z)
    };
}
