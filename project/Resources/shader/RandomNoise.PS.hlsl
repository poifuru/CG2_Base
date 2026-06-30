#include "Fullscreen.hlsli"

struct RandomNoise
{
    float time; // 時間経過（ノイズのパターンを毎フレーム変える用）
    float intensity; // ノイズの強さ (0.0 ～ 1.0)
    float2 padding; // 16バイトアライメント用のパディング
};

ConstantBuffer<RandomNoise> gNoise : register(b0);
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET;
};

float rand2dTo1d(float2 value, float2 dotDir = float2(12.9898, 78.233))
{
    float2 smallValue = sin(value);
    float random = dot(smallValue, dotDir);
    random = frac(sin(random) * 143758.5453);
    return random;
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
     // 元の画面の色を取得
    float4 texColor = gTexture.Sample(gSampler, input.texcoord);
    
    // 乱数生成。引数にtexcoordを渡す
    float random = rand2dTo1d(input.texcoord * gNoise.time);
    
    // ノイズを掛け合わせた色を作る
    float3 noiseAppliedColor = texColor.rgb * random;
    
    // 色にする(intensityを使って補間)
    output.color.rgb = lerp(texColor.rgb, noiseAppliedColor, gNoise.intensity);
    output.color.a = 1.0f;
    
    return output;
}