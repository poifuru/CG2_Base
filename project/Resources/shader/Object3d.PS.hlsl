#include "Object3d.hlsli"

//***構造体やレジスターの定義***//
struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

struct MaterialData
{
    float4 color;
    float4x4 uvTransform;
    float roughness; // 粗さ
    float metallic; // 金属度
    float environmentCoefficient;	// 環境係数
    int enableLighting;
    float time;
};

struct LightCount
{
    int dirLight;
    int pointLight;
    int spotLight;
    int rectLight;
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

struct RectLight
{
    float4 color;
    float3 position; // ライトの中心座標
    float intensity;
    float3 direction; // ライトの正面方向（法線）
    float padding1;
    float2 size; // Width(幅) と Height(高さ)
    float padding2[2];
    float3 right; // ライトの右方向ベクトル
    float padding;
    float3 up; // ライトの上方向ベクトル
    float decay; // 距離による減衰率（PointLightと同様）
};

// 【Slot 4 (space0)】: カメラバッファ
struct Camera
{
    float3 worldPosition;
    float padding;
};
ConstantBuffer<Camera> gCamera : register(b2, space0);

// 【Slot 5 (space0)】: ライトデータ一括
struct AllLightData
{
    LightCount count;
    DirectionalLight dirLights[50];
    PointLight pointLights[50];
    SpotLight spotLights[50];
    RectLight rectLights[50];
};
ConstantBuffer<AllLightData> gLightData : register(b3, space0);

// 【Slot 1 (space0)】: インデックス定数
struct PushIndices
{
    uint materialIndex;
    uint textureIndex;
};
ConstantBuffer<PushIndices> g_Indices : register(b1, space0);

// 【Slot 2 (space1)】: 巨大SRVヒープ (マテリアルデータ)
StructuredBuffer<MaterialData> g_BindlessBuffers[] : register(t0, space1);

// 【Slot 3 (space2)】: 巨大SRVヒープ (テクスチャ)
Texture2D g_Textures[] : register(t0, space2);

// サンプラー (s0, space0)
SamplerState gSampler : register(s0, space0);
//******//

static const float PI = 3.14159265359f;

float GeometrySchlickGGX(float NdotV, float k)
{
    return NdotV / (NdotV * (1.0f - k) + k);
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = saturate(dot(N, H));
    float denom = (NdotH * NdotH * (a2 - 1.0f) + 1.0f);
    return a2 / (PI * denom * denom);
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float r = (roughness + 1.0f);
    float k = (r * r) / 8.0f;
    
    float NdotV = saturate(dot(N, V));
    float NdotL = saturate(dot(N, L));
    
    float ggx2 = GeometrySchlickGGX(NdotV, k);
    float ggx1 = GeometrySchlickGGX(NdotL, k);
    
    return ggx1 * ggx2;
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0f - F0) * pow(saturate(1.0f - cosTheta), 5.0f);
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    // バインドレスマテリアルの取得
    uint matIdx = g_Indices.materialIndex;
    MaterialData myMaterial = g_BindlessBuffers[matIdx][0];
    
    // バインドレステクスチャのサンプリング
    uint texIdx = g_Indices.textureIndex;
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), myMaterial.uvTransform);
    float4 textureColor = g_Textures[texIdx].Sample(gSampler, transformedUV.xy);
    
    if (textureColor.a <= 0.5f || myMaterial.color.a == 0.0f) { discard; }
    
    float3 totalDiffuse = float3(0, 0, 0);
    float3 totalSpecular = float3(0, 0, 0);
    float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
    
    // DirectionalLight
    for (int i = 0; i < gLightData.count.dirLight; ++i)
    {
        float3 L = normalize(-gLightData.dirLights[i].direction);
        float3 N = normalize(input.normal);
        float3 V = toEye;
        float3 H = normalize(V + L);

        float3 lightColor = gLightData.dirLights[i].color.rgb;
        float intensity = gLightData.dirLights[i].intensity;

        float roughness = saturate(myMaterial.roughness);
        float metallic = saturate(myMaterial.metallic);
        float3 albedo = myMaterial.color.rgb * textureColor.rgb;
        float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo, metallic);

        float D = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        float3 F = FresnelSchlick(saturate(dot(H, V)), F0);

        float3 specular = (D * G * F) / (4.0f * saturate(dot(N, V)) * saturate(dot(N, L)) + 0.0001f);
        float3 kD = (float3(1.0f, 1.0f, 1.0f) - F) * (1.0f - metallic);
        float3 diffuse = kD * albedo / PI;

        totalDiffuse += (diffuse + specular) * lightColor * intensity * saturate(dot(N, L));
    }
    
    // PointLight
    for (int j = 0; j < gLightData.count.pointLight; ++j)
    {
        float3 direction = input.worldPosition - gLightData.pointLights[j].position;
        float distance = length(direction);
        float3 L = normalize(-direction);
        float3 N = normalize(input.normal);
        float3 V = toEye;
        float3 H = normalize(V + L);

        float attenuation = pow(saturate(1.0f - (distance / gLightData.pointLights[j].radius)), gLightData.pointLights[j].decay);
        float3 lightColor = gLightData.pointLights[j].color.rgb;
        float intensity = gLightData.pointLights[j].intensity * attenuation;

        float roughness = saturate(myMaterial.roughness);
        float metallic = saturate(myMaterial.metallic);
        float3 albedo = myMaterial.color.rgb * textureColor.rgb;

        float3 F0 = float3(0.04f, 0.04f, 0.04f);
        F0 = lerp(F0, albedo, metallic);

        float D = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        float3 F = FresnelSchlick(saturate(dot(H, V)), F0);

        float3 numerator = D * G * F;
        float denominator = 4.0f * saturate(dot(N, V)) * saturate(dot(N, L)) + 0.0001f;
        float3 specular = numerator / denominator;

        float3 kS = F;
        float3 kD = (float3(1.0f, 1.0f, 1.0f) - kS) * (1.0f - metallic);
        float3 diffuse = kD * albedo / PI;

        float nDotL = saturate(dot(N, L));
        totalDiffuse += (diffuse + specular) * lightColor * intensity * nDotL;
    }
    
    // SpotLight
    for (int k = 0; k < gLightData.count.spotLight; ++k)
    {
        float3 direction = input.worldPosition - gLightData.spotLights[k].position;
        float distance = length(direction);
        float3 L = normalize(-direction);
        float3 N = normalize(input.normal);
        float3 V = toEye;
        float3 H = normalize(V + L);

        float attenuation = pow(saturate(1.0f - (distance / gLightData.spotLights[k].distance)), gLightData.spotLights[k].decay);
        float cosToPos = dot(-L, normalize(gLightData.spotLights[k].direction));
        float spotFactor = saturate((cosToPos - gLightData.spotLights[k].cosAngle) / (1.0f - gLightData.spotLights[k].cosAngle));
        float intensity = gLightData.spotLights[k].intensity * attenuation * spotFactor;

        float3 lightColor = gLightData.spotLights[k].color.rgb;

        float roughness = saturate(myMaterial.roughness);
        float metallic = saturate(myMaterial.metallic);
        float3 albedo = myMaterial.color.rgb * textureColor.rgb;
        float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo, metallic);

        float D = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        float3 F = FresnelSchlick(saturate(dot(H, V)), F0);

        float3 specular = (D * G * F) / (4.0f * saturate(dot(N, V)) * saturate(dot(N, L)) + 0.0001f);
        float3 kD = (float3(1.0f, 1.0f, 1.0f) - F) * (1.0f - metallic);
        float3 diffuse = kD * albedo / PI;

        totalDiffuse += (diffuse + specular) * lightColor * intensity * saturate(dot(N, L));
    }
    
    // RectLight
    for (int l = 0; l < gLightData.count.rectLight; ++l)
    {
        float3 halfW = gLightData.rectLights[l].right * (gLightData.rectLights[l].size.x * 0.5f);
        float3 halfH = gLightData.rectLights[l].up * (gLightData.rectLights[l].size.y * 0.5f);
        
        float3 p[4];
        p[0] = gLightData.rectLights[l].position - halfW - halfH; // 左下
        p[1] = gLightData.rectLights[l].position + halfW - halfH; // 右下
        p[2] = gLightData.rectLights[l].position + halfW + halfH; // 右上
        p[3] = gLightData.rectLights[l].position - halfW + halfH; // 左上

        float3 v[4];
        for (int idx = 0; idx < 4; idx++)
        {
            v[idx] = normalize(p[idx] - input.worldPosition);
        }

        float illuminance = 0.0f;
        illuminance += acos(saturate(dot(v[0], v[1])));
        illuminance += acos(saturate(dot(v[1], v[2])));
        illuminance += acos(saturate(dot(v[2], v[3])));
        illuminance += acos(saturate(dot(v[3], v[0])));
        
        illuminance /= (2.0f * PI);

        float3 distVec = input.worldPosition - gLightData.rectLights[l].position;
        float distance = length(distVec);
        float attenuation = pow(saturate(1.0f - (distance / 20.0f)), gLightData.rectLights[l].decay);

        float3 N = normalize(input.normal);
        float3 L = normalize(-gLightData.rectLights[l].direction);
        float nDotL = saturate(dot(N, L));

        float3 albedo = myMaterial.color.rgb * textureColor.rgb;
        float3 diffuse = (albedo / PI) * gLightData.rectLights[l].color.rgb * gLightData.rectLights[l].intensity * illuminance * nDotL * attenuation;

        totalDiffuse += diffuse;
    }
    
    // 環境マップは未バインドのため、0.0で安全にフォールバック
    float4 environmentColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

    if (myMaterial.enableLighting == 0)
    {
        output.color = myMaterial.color * textureColor;
    }
    else
    {
        output.color.rgb = totalDiffuse + environmentColor.rgb;
        output.color.a = myMaterial.color.a * textureColor.a;
    }
    
    return output;
}