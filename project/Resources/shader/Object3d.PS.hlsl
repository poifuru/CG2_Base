#include "Object3d.hlsli"

//***構造体やレジスターの定義***//
struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

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

Texture2D<float4> gTexture : register(t0);

StructuredBuffer<DirectionalLight> gDirectionalLight : register(t1);

StructuredBuffer<PointLight> gPointLight : register(t2);

StructuredBuffer<SpotLight> gSpotLight : register(t3);

SamplerState gSampler : register(s0);
//******//

//***関数周りの定義***//
static const float PI = 3.14159265359f;

// ① 小さなパーツ：遮蔽率の計算
float GeometrySchlickGGX(float NdotV, float k)
{
    return NdotV / (NdotV * (1.0f - k) + k);
}

// D項: 物体の微細な凹凸による反射の広がり
float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = saturate(dot(N, H));
    float denom = (NdotH * NdotH * (a2 - 1.0f) + 1.0f);
    return a2 / (PI * denom * denom);
}

// G項: 凹凸による光の遮り（幾何減衰）
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    // 粗さから係数kを算出
    float r = (roughness + 1.0f);
    float k = (r * r) / 8.0f;
    
    float NdotV = saturate(dot(N, V));
    float NdotL = saturate(dot(N, L));
    
    // 視点からの遮蔽と、ライトからの遮蔽を掛け合わせる
    float ggx2 = GeometrySchlickGGX(NdotV, k);
    float ggx1 = GeometrySchlickGGX(NdotL, k);
    
    return ggx1 * ggx2;
}

// F項: 見る角度による反射率（フレネル）
float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0f - F0) * pow(saturate(1.0f - cosTheta), 5.0f);
}
//******//

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
        // 1. 基本的なベクトルと距離の計算
        float3 direction = input.worldPosition - gPointLight[j].position;
        float distance = length(direction);
        float3 L = normalize(-direction); // ライトへの方向
        float3 N = normalize(input.normal);
        float3 V = toEye;
        float3 H = normalize(V + L); // ハーフベクトル

        // 2. 減衰と強度の計算
        float attenuation = pow(saturate(1.0f - (distance / gPointLight[j].radius)), gPointLight[j].decay);
        float3 lightColor = gPointLight[j].color.rgb;
        float intensity = gPointLight[j].intensity * attenuation;

        // 3. PBRパラメータの設定 (後でgMaterialに追加すると便利でやんす！)
        float roughness = 0.3f; // 0.05〜1.0 (0だと0除算で壊れるので注意！)
        float metallic = 0.0f; // 0.0 (プラスチック) 〜 1.0 (金属)
        float3 albedo = gMaterial.color.rgb * textureColor.rgb;
        
        // F0: 垂直に入射した時の反射率
        // 非金属は 0.04 固定、金属は albedo の色をそのまま使うのがPBRのルールでやんす
        float3 F0 = float3(0.04f, 0.04f, 0.04f);
        F0 = lerp(F0, albedo, metallic);

        // 4. Cook-Torrance BRDF 各項の計算
        float D = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        float3 F = FresnelSchlick(saturate(dot(H, V)), F0);

        // 5. Specular (鏡面反射) の合成
        float3 numerator = D * G * F;
        float denominator = 4.0f * saturate(dot(N, V)) * saturate(dot(N, L)) + 0.0001f;
        float3 specular = numerator / denominator;

        // 6. Diffuse (拡散反射) の合成 (エネルギー保存)
        // 反射した分（kS）を引いた残りが拡散（kD）になるでやんす
        float3 kS = F;
        float3 kD = (float3(1.0f, 1.0f, 1.0f) - kS) * (1.0f - metallic);
        float3 diffuse = kD * albedo / PI;

        // 7. 最終的な加算
        float nDotL = saturate(dot(N, L));
        totalDiffuse += (diffuse + specular) * lightColor * intensity * nDotL;
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