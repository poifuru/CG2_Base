#include "Fullscreen.hlsli"

struct Vignette
{
    float2 center; // ビネットの中心座標 (通常は 0.5, 0.5)
    float innerRadius; // 暗くなり始める半径 (0.0 ～ 1.0)
    float outerRadius; // 完全に暗くなる半径 (0.0 ～ 1.0)
    float3 vignetteColor; // ビネットの色 (黒なら 0.0, 0.0, 0.0)
    float intensity; // ビネットの強度 (0.0 で効果なし, 1.0 で完全適用)
    float aspectRatio; // 画面のアスペクト比 (Width / Height)
};

ConstantBuffer<Vignette> gVignette : register(b0);
Texture2D<float4> gTexture : register(t0);
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
    float2 toCenter = input.texcoord - gVignette.center;
    toCenter.x *= gVignette.aspectRatio; // 横のスケールを補正して真円にする
    
    // 中心からの距離を計算
    float dist = length(toCenter);
    
    // smoothstep を用いて滑らかな減衰係数を計算
    // 指定した Inner から Outer にかけて 0.0 ～ 1.0 に変化する
    float vignetteFactor = smoothstep(gVignette.innerRadius, gVignette.outerRadius, dist);
    
    // 強度（Intensity）を適用
    vignetteFactor *= gVignette.intensity;
    
    // ビネット色との線形補間（lerp）
    // vignetteFactor が 1.0 に近づくほど g_VignetteColor になる
    output.color.rgb = lerp(output.color.rgb, gVignette.vignetteColor, vignetteFactor);
   
    return output;
}