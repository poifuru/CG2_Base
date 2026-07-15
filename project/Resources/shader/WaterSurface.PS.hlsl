#include "Object3d.hlsli"
#include "Light.hlsli"
#include "MaterialData.hlsli"

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

struct Camera
{
    float3 worldPosition;
    float padding;
};

struct PushIndices
{
    uint materialIndex;
    uint textureIndex;
};

// 【Slot 1 (space0)】: インデックス定数
ConstantBuffer<PushIndices> g_Indices : register(b1, space0);

// 【Slot 2 (space1)】: 巨大SRVヒープ (マテリアルデータ)
StructuredBuffer<MaterialData> g_BindlessBuffers[] : register(t0, space1);

// 【Slot 3 (space2)】: 巨大SRVヒープ (テクスチャ)
Texture2D g_Textures[] : register(t0, space2);

// 【Slot 4 (space0)】: カメラバッファ
ConstantBuffer<Camera> gCamera : register(b2, space0);

// 【Slot 5 (space0)】: ライトデータ一括
ConstantBuffer<AllLightData> gLightData : register(b3, space0);

// サンプラー (s0, space0)
SamplerState gSampler : register(s0, space0);


PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    // マテリアルデータの取得
    uint matIndex = g_Indices.materialIndex;
    MaterialData material = g_BindlessBuffers[matIndex][0];
    
    // 水面タイリングの細かさ(グリッドに対してテクスチャを何回繰り返すか)
    float2 uvScale = float2(4.0f, 4.0f);
    
    // 1枚目と2枚目のスクロール方向・速さ(逆方向や斜めに流すと複雑に見える)
    float2 speed1 = float2(0.02f, 0.01f);
    float2 speed2 = float2(-0.015f, 0.025f);
    
    float2 uv1 = input.texcoord * uvScale + speed1 * material.time;
    float2 uv2 = input.texcoord * uvScale + speed2 * material.time;
    
    // テクスチャを取得
    uint texIndex = g_Indices.textureIndex; // 法線マップのインデックス
    
    // 2回サンプリングする
    float3 normalMap1 = g_Textures[texIndex].Sample(gSampler, uv1).rgb;
    float3 normalMap2 = g_Textures[texIndex].Sample(gSampler, uv2).rgb;
    
    // [0, 1] 範囲から [-1, 1] にデコード
    float3 normal1 = normalMap1 * 2.0f - 1.0f;
    float3 normal2 = normalMap2 * 2.0f - 1.0f;
    
    // 2つの法線を合成して normalize (z成分を掛け合わせて、上向きをキープしながらブレンド)
    float3 blendedNormal = normalize(float3(normal1.xy + normal2.xy, normal1.z * normal2.z));
    
    // 簡易TBN（スクリーン座標の微分を利用して、法線マップをワールド座標系に合わせる）
    float3 N = normalize(input.normal); // 頂点シェーダーからきたワールド法線
    float3 dp1 = ddx(input.worldPosition);
    float3 dp2 = ddy(input.worldPosition);
    float2 duv1 = ddx(input.texcoord);
    float2 duv2 = ddy(input.texcoord);

    float3 dp2perp = cross(dp2, N);
    float3 dp1perp = cross(N, dp1);
    float3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    float3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    float invmax = rsqrt(max(dot(T, T), dot(B, B)));
    float3x3 TBN = float3x3(T * invmax, B * invmax, N);

    // 最終的なワールド空間の法線を計算
    float3 worldNormal = normalize(mul(blendedNormal, TBN));
    
    // 最終的に出力する色
    float3 finalColor = float3(0.0f, 0.0f, 0.0f);
    
    // 全てのライト情報を取得
    AllLightData alllights = gLightData;
    
    // テクスチャの色を取得
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), material.uvTransform);
    float4 textureColor = g_Textures[texIndex].Sample(gSampler, transformedUV.xy);
    
    // 視点のベクトル
    float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
    
    // 水色のダミーカラーマップ
    float4 waterBaseColor = float4(0.0f, 0.35f, 0.6f, 1.0f);
    
    finalColor = UpdateLights(alllights, material, input, waterBaseColor, toEye, worldNormal);
    
    // 最終出力
    output.color.rgb = finalColor;
    output.color.a = material.color.a * textureColor.a;
    
    return output;
}