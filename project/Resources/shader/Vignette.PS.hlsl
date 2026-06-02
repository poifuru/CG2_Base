#include "Fullscreen.hlsli"

struct VignetingData
{
    float intensity; // エフェクトの強度（0.0 〜 1.0など）
    float power; // 時間（ノイズのアニメーションや画面の揺れに使う）
    float dummy1; // 16バイトアライメント用のパディング
    float dummy2;
};

ConstantBuffer<VignetingData> gVignetingData : register(b0);
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
    
    // 周囲を0に、中心になるほど明るくなるように計算で調整
    float2 correct = input.texcoord * (1.0f - input.texcoord.yx);
    // correctだけで計算すると中心の最大値が0.0625で暗すぎるのでintensityで調整
    float vignette = correct.x * correct.y * gVignetingData.intensity;
    // 係数
    vignette = saturate(pow(vignette, gVignetingData.power));
    // 係数として乗算
    output.color.rgb *= vignette;
   
    return output;
}