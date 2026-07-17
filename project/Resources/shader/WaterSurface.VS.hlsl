#include "Object3d.hlsli"

struct TransformationMatrix
{
    float4x4 WVP;
    float4x4 World;
    float4x4 WorldInverseTranspose;
};

struct WaterSurfaceInfo
{
    float amplitude; // 振幅(A)
    float frequency; // 周波数(w)
    float steepness; // 険しさ(Q)
    float padding;
    float2 direction; // 方向(D)
    float2 padding1;
};

struct WaterSurface
{
    WaterSurfaceInfo waves[4]; // 4つの波の情報（32バイト * 4 = 128バイト）
    float time; // 時間
    int numActiveWaves; // 現在有効にする波の数（1〜4）
    float nearFadeDistance;
    float farFadeDistance;
};

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);
ConstantBuffer<WaterSurface> gWaterSurface : register(b4);

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    
    // 元の頂点座標をコピー
    float3 position = input.position.xyz;
    
     // 複数の波の変位を累積する変数
    float3 waveOffset = float3(0.0f, 0.0f, 0.0f);
    
    // 接線と従法線の初期値(変形前はそれぞれX方向、Z方向)
    float3 tangent = float3(1.0f, 0.0f, 0.0f);
    float3 binormal = float3(0.0f, 0.0f, 1.0f);
    
    for (int i = 0; i < gWaterSurface.numActiveWaves; ++i)
    {
        // ゲルストナー波のパラメータ
        float A = gWaterSurface.waves[i].amplitude;
        float w = gWaterSurface.waves[i].frequency;
        float Q = gWaterSurface.waves[i].steepness;
        float t = gWaterSurface.time;
        float2 D = gWaterSurface.waves[i].direction;
    
        // cos / sin の中身を計算
        float theta = w * dot(D, position.xz) + t;
        
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);
    
        // 水平方向(x, z)と高さ方向(y)のズレを計算して適用
        waveOffset.x += Q * A * D.x * cos(theta);
        waveOffset.z += Q * A * D.y * cos(theta);
        waveOffset.y += A * sin(theta);
        
        // 接線ベクトル（X方向の偏微分）の累積
        tangent.x -= Q * A * D.x * D.x * w * sinTheta;
        tangent.y += A * D.x * w * cosTheta;
        tangent.z -= Q * A * D.x * D.y * w * sinTheta;
        
        // 従法線ベクトル（Z方向の偏微分）の累積
        binormal.x -= Q * A * D.x * D.y * w * sinTheta;
        binormal.y += A * D.y * w * cosTheta;
        binormal.z -= Q * A * D.y * D.y * w * sinTheta;
    }
    
     // 累積した変位を適用
    position += waveOffset;
    
    // w成分(1.0f)を新たに追加して4次元ベクトルを完成させる
    float4 finalPosition = float4(position, 1.0f);
    
    // 変形後の座標をWVP変換
    output.position = mul(finalPosition, gTransformationMatrix.WVP);
    output.texcoord = input.texcoord;
    
    // 二つのベクトルの外積から法線を計算して正規化する
    // 左手系において、binormal × tangent で上向きの法線が求まる
    float3 localNormal = normalize(cross(binormal, tangent));
    
    // 法線をワールド空間に変換してピクセルシェーダーに渡す
    float3 worldNormal = mul(localNormal, (float3x3) gTransformationMatrix.WorldInverseTranspose);
    output.normal = normalize(worldNormal);
    
    output.worldPosition = mul(finalPosition, gTransformationMatrix.World).xyz;
    
    return output;
}