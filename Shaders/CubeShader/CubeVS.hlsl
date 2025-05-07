cbuffer VertexCB : register(b0)
{
    matrix Transformation;
    matrix WorldMatrix;
    matrix ViewMatrix;
    matrix ProjectionMatrix;
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

    float4 pos = float4(input.position, 1.0f);

    pos = mul(Transformation, pos);
    pos = mul(WorldMatrix, pos);
    pos = mul(ViewMatrix, pos);
    pos = mul(ProjectionMatrix, pos);

    output.position = pos;
    output.color = input.color;
    return output;
}
