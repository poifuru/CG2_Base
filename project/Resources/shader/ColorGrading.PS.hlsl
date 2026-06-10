#include "Fullscreen.hlsli"

struct PostProcessData
{
    float intensity; // エフェクトの強度（0.0 〜 1.0など）
    float sepiaAmount; // セピア調にする強さ(0.0 : 完全な白黒, 1.0 : セピア)
    float2 padding0;

    float3 tintColor;
    float padding1;
};

ConstantBuffer<PostProcessData> gPostProcessData : register(b0);
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput {
    float4 color : SV_TARGET;
};

PixelShaderOutput main(VertexShaderOutput input) {
    PixelShaderOutput output;
 
    float4 originalColor = gTexture.Sample(gSampler, input.texcoord); 
    
    // 輝度を計算
    float value = dot(originalColor.rgb, float3(0.2125f, 0.7154f, 0.0721f));
    float3 grayColor = float3(value, value, value);
    
    // intensityを使ってブレンドする
    output.color.rgb = lerp(originalColor.rgb, grayColor, gPostProcessData.intensity);
    
    // アルファ値は元の色をそのまま維持
    output.color.a = originalColor.a;
   
    return output;
}