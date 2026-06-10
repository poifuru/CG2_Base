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
 
    // 元の画面の色をサンプリング
    float4 originalColor = gTexture.Sample(gSampler, input.texcoord);
    
    // 標準的な輝度（白黒）を計算
    float value = dot(originalColor.rgb, float3(0.2125f, 0.7154f, 0.0721f));
    float3 grayColor = float3(value, value, value);
    
    // 輝度に対して tintColor を掛け合わせて調色された色を作る
    // 単に掛けるだけでなく、人間の目が認識しやすいように輝度(value)をベースに着色する
    float3 tintedColor = grayColor * gPostProcessData.tintColor;
    
    // 純粋な白黒と調色された色を sepiaAmount でブレンドする
    // これにより、sepiaAmount=0で白黒、1でセピア（またはtintColorの色）になる
    float3 finalEffectColor = lerp(grayColor, tintedColor, gPostProcessData.sepiaAmount);
    
    // 最後に、元の画面の色と、完成したエフェクトの色を intensity でブレンドする
    output.color.rgb = lerp(originalColor.rgb, finalEffectColor, gPostProcessData.intensity);
    
    // アルファ値は元の色をそのまま維持
    output.color.a = originalColor.a;
   
    return output;
}