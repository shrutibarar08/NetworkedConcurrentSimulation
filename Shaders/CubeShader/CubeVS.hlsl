cbuffer VertexCB : register(b0)
{
    matrix Transformation;
    matrix ViewMatrix;
    matrix ProjectionMatrix;
    matrix WorldMatrix;
};

struct VSInput
{
    float3 position : POSITION;
    float4 color : COLOR;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    float4 pos = mul(float4(input.position, 1.0f), Transformation);
    pos = mul(pos, WorldMatrix);
    pos = mul(pos, ViewMatrix);
    pos = mul(pos, ProjectionMatrix);
    output.position = pos;
    output.color = input.color;
    return output;
}
