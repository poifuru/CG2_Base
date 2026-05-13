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
    float roughness; // 粗さ
    float metallic; // 金属度
    float environmentCoefficient;	// 環境係数
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
    float2 size; // Width(幅) と Height(高さ)
    float3 right; // ライトの右方向ベクトル
    float padding;
    float3 up; // ライトの上方向ベクトル
    float decay; // 距離による減衰率（PointLightと同様）
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

StructuredBuffer<RectLight> gRectLight : register(t4);

TextureCube<float4> gEnvironmentTexture : register(t5);

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
    if (textureColor.a <= 0.5f || output.color.a == 0.0f)
    {
        discard;
    }
    
    //lightingの準備
    float3 totalDiffuse = float3(0, 0, 0);
    float3 totalSpecular = float3(0, 0, 0);
    float3 toEye = normalize(gCamera.worldPosition - input.worldPosition); //Cameraへの方向を算出
    
    //DirectionalLightの計算
    for (int i = 0; i < gLightCount.dirLight; ++i)
    {
        float3 L = normalize(-gDirectionalLight[i].direction); // ライトへの方向
        float3 N = normalize(input.normal);
        float3 V = toEye;
        float3 H = normalize(V + L);

        float3 lightColor = gDirectionalLight[i].color.rgb;
        float intensity = gDirectionalLight[i].intensity;

        // PBRパラメータ
        float roughness = saturate(gMaterial.roughness);
        float metallic = saturate(gMaterial.metallic);
        float3 albedo = gMaterial.color.rgb * textureColor.rgb;
        float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo, metallic);

        // BRDF計算
        float D = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        float3 F = FresnelSchlick(saturate(dot(H, V)), F0);

        float3 specular = (D * G * F) / (4.0f * saturate(dot(N, V)) * saturate(dot(N, L)) + 0.0001f);
        float3 kD = (float3(1.0f, 1.0f, 1.0f) - F) * (1.0f - metallic);
        float3 diffuse = kD * albedo / PI;

        totalDiffuse += (diffuse + specular) * lightColor * intensity * saturate(dot(N, L));
    }
    
    //PointLightの計算
    for (int j = 0; j < gLightCount.pointLight; ++j)
    {
        //基本的なベクトルと距離の計算
        float3 direction = input.worldPosition - gPointLight[j].position;
        float distance = length(direction);
        float3 L = normalize(-direction); // ライトへの方向
        float3 N = normalize(input.normal);
        float3 V = toEye;
        float3 H = normalize(V + L); // ハーフベクトル

        //減衰と強度の計算
        float attenuation = pow(saturate(1.0f - (distance / gPointLight[j].radius)), gPointLight[j].decay);
        float3 lightColor = gPointLight[j].color.rgb;
        float intensity = gPointLight[j].intensity * attenuation;

        //マテリアル設定をgMaterialから取得
        float roughness = saturate(gMaterial.roughness);
        float metallic = saturate(gMaterial.metallic);
        float3 albedo = gMaterial.color.rgb * textureColor.rgb;

        //F0の決定（PBRの基本ルール）
        float3 F0 = float3(0.04f, 0.04f, 0.04f);
        F0 = lerp(F0, albedo, metallic);

        //Cook-Torrance BRDF 各項の計算
        float D = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        float3 F = FresnelSchlick(saturate(dot(H, V)), F0);

        //Specular (鏡面反射) の合成
        float3 numerator = D * G * F;
        float denominator = 4.0f * saturate(dot(N, V)) * saturate(dot(N, L)) + 0.0001f;
        float3 specular = numerator / denominator;

        //Diffuse (拡散反射) の合成 (エネルギー保存)
        // 反射した分（kS）を引いた残りが拡散（kD）になる
        float3 kS = F;
        float3 kD = (float3(1.0f, 1.0f, 1.0f) - kS) * (1.0f - metallic);
        float3 diffuse = kD * albedo / PI;

        //最終的な加算
        float nDotL = saturate(dot(N, L));
        totalDiffuse += (diffuse + specular) * lightColor * intensity * nDotL;
    }
    
    //SpotLightの計算
    for (int k = 0; k < gLightCount.spotLight; ++k)
    {
        float3 direction = input.worldPosition - gSpotLight[k].position;
        float distance = length(direction);
        float3 L = normalize(-direction);
        float3 N = normalize(input.normal);
        float3 V = toEye;
        float3 H = normalize(V + L);

        // 距離減衰
        float attenuation = pow(saturate(1.0f - (distance / gSpotLight[k].distance)), gSpotLight[k].decay);

        // 角度減衰
        float cosToPos = dot(-L, normalize(gSpotLight[k].direction));
        float spotFactor = saturate((cosToPos - gSpotLight[k].cosAngle) / (1.0f - gSpotLight[k].cosAngle));
        float intensity = gSpotLight[k].intensity * attenuation * spotFactor;

        float3 lightColor = gSpotLight[k].color.rgb;

        // PBRパラメータ
        float roughness = saturate(gMaterial.roughness);
        float metallic = saturate(gMaterial.metallic);
        float3 albedo = gMaterial.color.rgb * textureColor.rgb;
        float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo, metallic);

        // BRDF計算
        float D = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        float3 F = FresnelSchlick(saturate(dot(H, V)), F0);

        float3 specular = (D * G * F) / (4.0f * saturate(dot(N, V)) * saturate(dot(N, L)) + 0.0001f);
        float3 kD = (float3(1.0f, 1.0f, 1.0f) - F) * (1.0f - metallic);
        float3 diffuse = kD * albedo / PI;

        totalDiffuse += (diffuse + specular) * lightColor * intensity * saturate(dot(N, L));
    }
    
    //RectLightの計算
    for (int l = 0; l < gLightCount.rectLight; ++l)
    {
       //ライトの4隅の座標を計算
        float3 halfW = gRectLight[l].right * (gRectLight[l].size.x * 0.5f);
        float3 halfH = gRectLight[l].up * (gRectLight[l].size.y * 0.5f);
        
        float3 p[4];
        p[0] = gRectLight[l].position - halfW - halfH; // 左下
        p[1] = gRectLight[l].position + halfW - halfH; // 右下
        p[2] = gRectLight[l].position + halfW + halfH; // 右上
        p[3] = gRectLight[l].position - halfW + halfH; // 左上

        //各頂点への方向ベクトルを算出
        float3 v[4];
        for (int idx = 0; idx < 4; idx++)
        {
            v[idx] = normalize(p[idx] - input.worldPosition);
        }

        //面光源の強さを計算（簡易積分）
        //隣り合う頂点とのなす角を足していくことで、面としての影響力を出す
        float illuminance = 0.0f;
        illuminance += acos(saturate(dot(v[0], v[1])));
        illuminance += acos(saturate(dot(v[1], v[2])));
        illuminance += acos(saturate(dot(v[2], v[3])));
        illuminance += acos(saturate(dot(v[3], v[0])));
        
        //正規化（面光源っぽく調整）
        illuminance /= (2.0f * PI);

        //距離による減衰（中心からの距離で代用）
        float3 distVec = input.worldPosition - gRectLight[l].position;
        float distance = length(distVec);
        float attenuation = pow(saturate(1.0f - (distance / 20.0f)), gRectLight[l].decay); // 20.0fは有効範囲

        //拡散反射の計算
        float3 N = normalize(input.normal);
        float3 L = normalize(-gRectLight[l].direction); // ライトの正面
        float nDotL = saturate(dot(N, L));

        float3 albedo = gMaterial.color.rgb * textureColor.rgb;
        float3 diffuse = (albedo / PI) * gRectLight[l].color.rgb * gRectLight[l].intensity * illuminance * nDotL * attenuation;

        totalDiffuse += diffuse;
    }

    // 環境マップ用の処理
    float3 cameraToPosition = normalize(input.worldPosition - gCamera.worldPosition);
    float3 reflectedVector = reflect(cameraToPosition, normalize(input.normal));
    float3 environmentColor = gEnvironmentTexture.Sample(gSampler, reflectedVector).rgb;
    environmentColor.rgb *= gMaterial.environmentCoefficient;
    
    //最終出力の分岐
    if (gMaterial.enableLighting == 0)
    {
        output.color = gMaterial.color * textureColor;
    }
    else
    {
    //PBRの結果を表示
        output.color.rgb = totalDiffuse + environmentColor; // totalDiffuseの中にspecularも加算済みの場合
        output.color.a = gMaterial.color.a * textureColor.a;
    }
    
    return output;
}