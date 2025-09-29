#include "Object3d.hlsli"

struct Material
{
    float32_t4 color;
    int32_t enableLighting;
    float32_t4x4 uvTransform;
};

struct DirectionalLight
{
    float32_t4 color;
    float32_t3 direction;
    float intensity;
};

ConstantBuffer<DirectionalLight> gDirectionalLight : register(b3);

ConstantBuffer<Material> gMaterial : register(b1);  

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);   

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    // ■ デバッグ用法線表示：ここに入れて早期リターンする
    //{
    //    float3 n = normalize(input.normal);
    //    float3 normalColor = n * 0.5f + 0.5f; // [-1,1] -> [0,1]
    //    output.color = float4(normalColor, 1.0f);
    //    return output; // これで法線だけ見れる
    //}
    
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    output.color = gMaterial.color * textureColor;
    
    float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
    //Lighttingの計算	
    if (gMaterial.enableLighting == 1)  //Lightingする場合
    {
        float cos = saturate(dot(normalize(input.normal), -gDirectionalLight.direction));
        output.color = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;
    }
    else if (gMaterial.enableLighting == 2)
    {
        float NdotL = dot(normalize(input.normal), -normalize(gDirectionalLight.direction));
        float halfLambert = saturate(NdotL * 0.5 + 0.5); // 0〜1にマップ
        output.color = gMaterial.color * textureColor * gDirectionalLight.color * halfLambert * gDirectionalLight.intensity;
    }
    else    //Lightingしない場合
    {
        output.color = gMaterial.color * textureColor;
    }
    
    return output;
    
    //texcoordを見たいときの実験用
    //PixelShaderOutput output;

    // UVをそのまま色として出力する（確認用）
    //output.color = float4(input.texcoord, 0.0f, 1.0f);

    //return output;
}