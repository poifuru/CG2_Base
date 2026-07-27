struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float alpha : TEXCOORD1;
};

struct PushIndices
{
    uint materialIndex;
    uint textureIndex;
};

// 【Slot 1 (space0)】: PushConstantsからインデックスを受け取る
ConstantBuffer<PushIndices> g_Indices : register(b1, space0);

// 【Slot 3 (space2)】: バインドレステクスチャ配列
Texture2D g_Textures[] : register(t0, space2);

// サンプラー (s0, space0)
SamplerState gSampler : register(s0, space0);

float4 main(PixelShaderInput input) : SV_TARGET
{
     // PushConstantsからテクスチャのインデックスを取得
    uint texIdx = g_Indices.textureIndex;
    
    // バインドレステクスチャ配列からサンプリング
    float4 texColor = g_Textures[texIdx].Sample(gSampler, input.uv);
    
    // テクスチャのアルファ値 × 頂点時間経過のアルファ値
    float finalAlpha = texColor.a * input.alpha;
    
    return float4(texColor.rgb, finalAlpha);
}