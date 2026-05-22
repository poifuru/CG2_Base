struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct Material
{
    float4 color;
    float4x4 uvTransform;
    float roughness;
    float metallic;
    float environmentCoefficient;
    int enableLighting;
};
ConstantBuffer<Material> gMaterial : register(b1); // SpriteRendererで送ってるやつ

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    // UVTransformを適用
    float4 uvSample = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    
    // テクスチャの色をサンプリング
    float4 textureColor = gTexture.Sample(gSampler, uvSample.xy);
    
    // テクスチャの色 × マテリアルの色（ImGuiでいじってるやつ）
    output.color = textureColor * gMaterial.color;
    
    return output;
}