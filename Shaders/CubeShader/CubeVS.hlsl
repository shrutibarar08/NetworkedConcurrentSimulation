cbuffer VertexCB : register(b0)
{
    matrix Transformation;
    matrix ViewMatrix;
    matrix ProjectionMatrix;
    matrix WorldMatrix; // Identity matrix, ignored

    float DeltaTime;
    int IsStatic;
    float2 Padding0;

    float4 AngularVelocity;
    float4 Velocity;
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
    float3 worldPos : TEXCOORD0;
};

VSOutput main(VSInput input)
{
    VSOutput output;

    float3 modelPos = input.position;

    // === Safe normalize velocity ===
    float3 velDir = Velocity.xyz;
    float speedSq = dot(velDir, velDir);
    if (speedSq < 0.0001f)
        velDir = float3(0.0f, 0.0f, 1.0f);
    else
        velDir = normalize(velDir);

    float3 angVel = AngularVelocity.xyz;
    float angSpeed = length(angVel);

    // === Tail stretch effect ===
    float3 fragDir = normalize(modelPos);
    float tailAlignment = saturate(dot(-velDir, fragDir));
    float stretch = tailAlignment * saturate(length(Velocity.xyz) / 10.0f) * 0.4f;
    modelPos += -velDir * stretch;

    // === Twist from angular velocity ===
    float angle = DeltaTime * angSpeed * 5.0f;
    float3 axis = (angSpeed > 0.0001f) ? normalize(angVel) : float3(0, 1, 0);

    // Rodrigues' rotation formula (cheap twist)
    float3 twistPos = modelPos;
    float3 crossAV = cross(axis, twistPos);
    float dotAV = dot(axis, twistPos);
    modelPos = twistPos * cos(angle) + crossAV * sin(angle) + axis * dotAV * (1 - cos(angle));

    // === Animated wobble pulse ===
    float wave = sin(dot(modelPos, float3(1.0f, 1.0f, 1.0f)) * 5.0f + DeltaTime * 8.0f);
    modelPos += normalize(modelPos) * wave * 0.03f;

    // Output to pixel shader
    output.worldPos = modelPos;

    float4 pos = float4(modelPos, 1.0f);
    pos = mul(Transformation, pos);
    pos = mul(ViewMatrix, pos);
    pos = mul(ProjectionMatrix, pos);

    output.position = pos;
    output.color = input.color;
    return output;
}
