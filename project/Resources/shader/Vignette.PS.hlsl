#include "Fullscreen.hlsli"

struct Vignette
{
    float4 centerAndRadius;      // xy: center, z: innerRadius, w: outerRadius
    float4 colorAndIntensity;    // xyz: color, w: intensity
    float4 aspectAndPadding;     // x: aspectRatio, yzw: padding
};

ConstantBuffer<Vignette> gVignette : register(b0);
Texture2D<float4> gTexture : register(t0, space2);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = gTexture.Sample(gSampler, input.texcoord);
    
    // アスペクト比を考慮したUV座標の調整（中心からのディレクション）
    float2 toCenter = input.texcoord - gVignette.centerAndRadius.xy;
    toCenter.x *= gVignette.aspectAndPadding.x; // 横のスケールを補正して真円にする
    
    // 中心からの距離を計算
    float dist = length(toCenter);
    
    // smoothstep を用いて滑らかな減衰係数を計算
    // 指定した Inner から Outer にかけて 0.0 ～ 1.0 に変化する
    float vignetteFactor = smoothstep(gVignette.centerAndRadius.z, gVignette.centerAndRadius.w, dist);
    
    // 強度（Intensity）を適用
    vignetteFactor *= gVignette.colorAndIntensity.w;
    
    // ビネット色との線形補間（lerp）
    // vignetteFactor が 1.0 に近づくほど colorAndIntensity.rgb になる
    output.color.rgb = lerp(output.color.rgb, gVignette.colorAndIntensity.rgb, vignetteFactor);
   
    return output;
}