#include "Fullscreen.hlsli"

Texture2D<float4> gTexture : register(t0, space2);
SamplerState gSampler : register(s0);

struct PixelShaderOutput {
    float4 color : SV_TARGET;
};

// ACES Filmic トーンマッピング関数
float3 ACESFilm(float3 color)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((color * (a * color + b)) / (color * (c * color + d) + e));
}

PixelShaderOutput main(VertexShaderOutput input) {
    PixelShaderOutput output;
 
    float4 color = gTexture.Sample(gSampler, input.texcoord); 
    
    // 簡易トーンマッピング (Reinhardトーンマップ)
    // 1.0を超えるハイライトを綺麗に 0.0 ~ 1.0 の範囲に収める
    color.rgb = ACESFilm(color.rgb);
    
    output.color = color;
   
    return output;
}