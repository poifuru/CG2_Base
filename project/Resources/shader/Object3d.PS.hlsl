#include "Object3d.hlsli"

struct Material
{
    float4 color;
    int enableLighting;
    float4x4 uvTransform;
    float shininess;
    bool isSpecular;
};

struct DirectionalLight
{
    float4 color;
    float3 direction;
    float intensity;
};

struct Camera
{
    float3 worldPosition;
};

ConstantBuffer<Material> gMaterial : register(b1);

ConstantBuffer<DirectionalLight> gDirectionalLight : register(b2);

ConstantBuffer<Camera> gCamera : register(b3);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    float4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    output.color = gMaterial.color * textureColor;
    
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
    float cos = saturate(dot(normalize(input.normal), -gDirectionalLight.direction));
    
    //Cameraへの方向を算出
    float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
    
    //textureのアルファ値が一定以下ならその後の処理をしない(2値抜き)
    if (textureColor.a <= 0.5f)
    {
        discard;
    }
    if (output.color.a == 0.0f)
    {
        discard;
    }
    
    //Lighttingの計算
    //ベースの拡散反射(diffuse)を計算
    float3 diffuse = float3(0, 0, 0);
    
    if (gMaterial.enableLighting == 1)  //ランバート反射
    {
        diffuse = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
    }
    else if (gMaterial.enableLighting == 2) //ハーフランバート反射
    {
        float NdotL = dot(normalize(input.normal), -normalize(gDirectionalLight.direction));
        float halfLambert = saturate(NdotL * 0.5 + 0.5); // 0〜1にマップ
        diffuse = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * halfLambert * gDirectionalLight.intensity;
    }
    else //Lightingしない場合
    {
        output.color = gMaterial.color * textureColor;
    }
       
    //鏡面反射
    float3 specular = float3(0, 0, 0);
    
    //マテリアル側でisSpecularが有効なら
    if (gMaterial.isSpecular == true)
    {
        //鏡面反射を計算する(Phong)
        //float3 reflectLight = reflect(gDirectionalLight.direction, normalize(input.normal));
        //float RdotE = dot(reflectLight, toEye);
        //float specularPow = pow(saturate(RdotE), gMaterial.shininess); //反射強度
        
        //鏡面反射を計算する(Blinn-Phong)
        float3 halfVector = normalize(-gDirectionalLight.direction + toEye);
        float NDotH = dot(normalize(input.normal), halfVector);
        float specularPow = pow(saturate(NDotH), gMaterial.shininess);
        
        //鏡面反射
        specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * gMaterial.color.rgb * cos;
    }
       
        //最後に両者を足す
    output.color.rgb = diffuse + specular;
        //アルファは今まで通り
    output.color.a = gMaterial.color.a * textureColor.a;
    
    return output;
}