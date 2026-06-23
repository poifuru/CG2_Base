#include "Fullscreen.hlsli"

struct RadialBlur
{
    float2 center;
    float blurWidth;
    int sampleCount;
};

ConstantBuffer<RadialBlur> gRadialBlur : register(b0);
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float2 uv = input.texcoord;
    
    // 中心から現在のピクセルへの方向ベクトル
    float2 direction = uv - gRadialBlur.center;
    
    float4 accumulatedColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // サンプル数
    int numSamples = gRadialBlur.sampleCount;
    if (numSamples <= 0) {
        numSamples = 1;
    }
    
    [loop]
    for (int i = 0; i < numSamples; ++i) {
        // 中心に向かって徐々にずらしながらサンプリング
        float scale = 1.0f - gRadialBlur.blurWidth * (float(i) / float(numSamples - 1));
        float2 sampleUV = gRadialBlur.center + direction * scale;
        
        accumulatedColor += gTexture.Sample(gSampler, sampleUV);
    }
    
    output.color = accumulatedColor / float(numSamples);
    return output;
}