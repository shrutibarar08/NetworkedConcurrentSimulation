cbuffer PixelCB : register(b0)
{
    float DeltaTime; // 4 bytes
    int IsStatic; // 4 bytes
    float2 Padding0; // 8 bytes (to align next float4)

    float4 AngularVelocity; // 16 bytes
    float4 Velocity; // 16 bytes
};


struct VSOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float3 worldPos : TEXCOORD0;
};

float4 main(VSOutput input) : SV_TARGET
{
    float3 baseColor = input.color.rgb;

    // === Safe velocity normalization ===
    float3 velDir = Velocity.xyz;
    float speedSq = dot(velDir, velDir);
    bool isMoving = speedSq > 0.0001f;

    velDir = isMoving ? normalize(velDir) : float3(0.0f, 0.0f, 1.0f);

    // === Direction from object center to pixel ===
    float3 fragDir = normalize(input.worldPos);

    // === Glow alignment and pulse ===
    float alignment = saturate(dot(-velDir, fragDir));
    float pulse = 0.4f + 0.2f * sin(DeltaTime * 10.0f); // reduced range: [0.2, 0.6]

    float glowIntensity = isMoving ? smoothstep(0.3f, 0.9f, alignment) * pulse : 0.0f;

    // === Anime tail color: soft yellow-orange glow (not pure white) ===
    float3 glowColor = float3(1.0f, 0.7f, 0.3f);

    // === Soft blend: mix instead of add ===
    float3 finalColor = lerp(baseColor, glowColor, glowIntensity);

    return float4(finalColor, 1.0f);
}
