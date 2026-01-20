#include "Object3d.hlsli"

struct Material
{
    float4 color;
    int enableLighting;
    float4x4 uvTransform;
    float shininess;
    bool isSpecular;
};

struct LightCount
{
    int dirLight;
    int pointLight;
    int spotLight;
};

struct DirectionalLight
{
    float4 color;
    float3 direction;
    float intensity;
};

struct PointLight
{
    float4 color;
    float3 position;
    float intensity;
    float radius; //ライトが届く最大距離
    float decay; //減衰率
    float padding[2];
};

struct SpotLight
{
    float4 color;
    float3 position;
    float intensity;
    float3 direction;
    float distance;
    float decay;
    float cosAngle;
    float padding[2];
};

struct Camera
{
    float3 worldPosition;
};

ConstantBuffer<Material> gMaterial : register(b1);

ConstantBuffer<Camera> gCamera : register(b2);

ConstantBuffer<LightCount> gLightCount : register(b3);

StructuredBuffer<DirectionalLight> gDirectionalLight : register(t1);

StructuredBuffer<PointLight> gPointLight : register(t2);

StructuredBuffer<SpotLight> gSpotLight : register(t3);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    //テクスチャサンプリングと基本色の決定
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
    //textureのアルファ値,出力カラーのアルファ値が一定以下ならその後の処理をしない(2値抜き)
    if (textureColor.a <= 0.5f || output.color.a == 0.0f) { discard; }
    
    //lightingの準備
    float3 totalDiffuse = float3(0, 0, 0);
    float3 totalSpecular = float3(0, 0, 0);
    float3 toEye = normalize(gCamera.worldPosition - input.worldPosition); //Cameraへの方向を算出
    
    //DirectionalLightの計算
    for (int i = 0; i < gLightCount.dirLight; ++i)
    {
        float3 lightDir = normalize(gDirectionalLight[i].direction);
        float3 lightColor = gDirectionalLight[i].color.rgb;
        float intensity = gDirectionalLight[i].intensity;

        // 拡散反射 (Diffuse)
        float cos = saturate(dot(normalize(input.normal), -lightDir));
        float3 dDiffuse = float3(0, 0, 0);

        if (gMaterial.enableLighting == 1)
        { // ランバート
            dDiffuse = gMaterial.color.rgb * textureColor.rgb * lightColor * cos * intensity;
        }
        else if (gMaterial.enableLighting == 2)
        { // ハーフランバート
            float halfLambert = saturate(dot(normalize(input.normal), -lightDir) * 0.5 + 0.5);
            dDiffuse = gMaterial.color.rgb * textureColor.rgb * lightColor * halfLambert * intensity;
        }

        // 鏡面反射 (Specular)
        float3 dSpecular = float3(0, 0, 0);
        if (gMaterial.isSpecular)
        {
            float3 halfVector = normalize(-lightDir + toEye);
            float NDotH = dot(normalize(input.normal), halfVector);
            float specularPow = pow(saturate(NDotH), gMaterial.shininess);
            dSpecular = lightColor * intensity * specularPow * gMaterial.color.rgb * cos;
        }

        totalDiffuse += dDiffuse;
        totalSpecular += dSpecular;
    }
    
    //PointLightの計算
    for (int j = 0; j < gLightCount.pointLight; ++j)
    {
        //ライトへの方向と距離を計算
        float3 direction = input.worldPosition - gPointLight[j].position;
        float distance = length(direction); // ライトとの距離
        float3 lightDir = normalize(direction); // 正規化して方向を出す

        //逆二乗則による減衰計算
        float attenuation = pow(saturate(1.0f - (distance / gPointLight[j].radius)), gPointLight[j].decay);
        
        float3 lightColor = gPointLight[j].color.rgb;
        float intensity = gPointLight[j].intensity * attenuation; // 減衰を乗算

        //拡散反射 (Diffuse)
        float cos = saturate(dot(normalize(input.normal), -lightDir));
        float3 pDiffuse = float3(0, 0, 0);

        if (gMaterial.enableLighting == 1) // ランバート
        {
            pDiffuse = gMaterial.color.rgb * textureColor.rgb * lightColor * cos * intensity;
        }
        else if (gMaterial.enableLighting == 2) // ハーフランバート
        {
            float halfLambert = saturate(dot(normalize(input.normal), -lightDir) * 0.5 + 0.5);
            pDiffuse = gMaterial.color.rgb * textureColor.rgb * lightColor * halfLambert * intensity;
        }

        //鏡面反射 (Specular)
        float3 pSpecular = float3(0, 0, 0);
        if (gMaterial.isSpecular)
        {
            float3 halfVector = normalize(-lightDir + toEye);
            float NDotH = dot(normalize(input.normal), halfVector);
            float specularPow = pow(saturate(NDotH), gMaterial.shininess);
            pSpecular = lightColor * intensity * specularPow * gMaterial.color.rgb * cos;
        }

        //全体の光に加算
        totalDiffuse += pDiffuse;
        totalSpecular += pSpecular;
    }
    
    //SpotLightの計算
    for (int k = 0; k < gLightCount.spotLight; ++k)
    {
        //基本的な方向と距離の計算
        float3 direction = input.worldPosition - gSpotLight[k].position;
        float distance = length(direction);
        float3 lightDir = normalize(direction);

        //距離による減衰
        float attenuation = pow(saturate(1.0f - (distance / gSpotLight[k].distance)), gSpotLight[k].decay);

        //角度による減衰 (Spotlight Factor)
        //ライトの向きと、ピクセルへの向きの余弦（cos）を計算
        float cosToPos = dot(lightDir, normalize(gSpotLight[k].direction));
        //指定された角度（cosAngle）より外側なら暗くする計算
        float spotFactor = saturate((cosToPos - gSpotLight[k].cosAngle) / (1.0f - gSpotLight[k].cosAngle));
        //秋口をなめらかにするためにここでもう一度 saturate して強度を出す
        float falloff = spotFactor * attenuation;

        float3 lightColor = gSpotLight[k].color.rgb;
        float intensity = gSpotLight[k].intensity * falloff;

        //拡散反射 (Diffuse)
        float cos = saturate(dot(normalize(input.normal), -lightDir));
        float3 sDiffuse = float3(0, 0, 0);

        if (gMaterial.enableLighting == 1) // ランバート
        {
            sDiffuse = gMaterial.color.rgb * textureColor.rgb * lightColor * cos * intensity;
        }
        else if (gMaterial.enableLighting == 2) // ハーフランバート
        {
            float halfLambert = saturate(dot(normalize(input.normal), -lightDir) * 0.5 + 0.5);
            sDiffuse = gMaterial.color.rgb * textureColor.rgb * lightColor * halfLambert * intensity;
        }

        //鏡面反射 (Specular)
        float3 sSpecular = float3(0, 0, 0);
        if (gMaterial.isSpecular)
        {
            float3 halfVector = normalize(-lightDir + toEye);
            float NDotH = dot(normalize(input.normal), halfVector);
            float specularPow = pow(saturate(NDotH), gMaterial.shininess);
            sSpecular = lightColor * intensity * specularPow * gMaterial.color.rgb * cos;
        }

        //全体の光に加算
        totalDiffuse += sDiffuse;
        totalSpecular += sSpecular;
    }
    
    // ライティングしない設定なら元の色を出す
    if (gMaterial.enableLighting == 0)
    {
        output.color = gMaterial.color * textureColor;
    }
    else
    {
        output.color.rgb = totalDiffuse + totalSpecular;
        output.color.a = gMaterial.color.a * textureColor.a;
    }
    
    return output;
}