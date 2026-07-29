#include "Fullscreen.hlsli"

struct RandomNoise
{
    float time;
    float intensity;
    float2 padding;
};

ConstantBuffer<RandomNoise> gRandomNoise : register(b0);
Texture2D<float4> gTexture : register(t0, space2);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET;
};

float random(float2 st)
{
    return frac(sin(dot(st.xy, float2(12.9898, 78.233))) * 43758.5453123);
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 originalColor = gTexture.Sample(gSampler, input.texcoord);
    
    // 画面座標と時間から擬似乱数を計算
    float noise = (random(input.texcoord + gRandomNoise.time) - 0.5f) * gRandomNoise.intensity;
    
    output.color = float4(originalColor.rgb + noise, originalColor.a);
    return output;
}