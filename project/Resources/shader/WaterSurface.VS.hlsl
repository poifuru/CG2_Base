#include "Object3d.hlsli"

struct TransformationMatrix
{
    float4x4 WVP;
    float4x4 World;
    float4x4 WorldInverseTranspose;
};

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    
    // 元の頂点座標をコピー
    float4 position = input.position;
    
    // ゲルストナー波のパラメータ（ハードコーディングでテスト）
    float A = 0.5f; // 振幅（波の高さ）
    float w = 0.5f; // 周波数（波の細かさ）
    float2 D = normalize(float2(1.0f, 1.0f)); // 波が進む方向（斜め45度）
    float Q = 0.5f; // 険しさ（0〜1の間で調整）
    float t = 0.0f; // 時間（まずは止めた状態にする。後で動かす）
    
    // cos / sin の中身を計算
    float theta = w * dot(D, position.xz) + t;
    
    // 水平方向(x, z)と高さ方向(y)のズレを計算して適用
    position.x += Q * A * D.x * cos(theta);
    position.z += Q * A * D.y * cos(theta);
    position.y += A * sin(theta);
    
    // 変形後の座標をWVP変換
    output.position = mul(position, gTransformationMatrix.WVP);
    output.texcoord = input.texcoord;
    
    // 法線は一旦そのまま
    float3 worldNormal = mul(input.normal, (float3x3) gTransformationMatrix.WorldInverseTranspose);
    output.normal = normalize(worldNormal);
    
    output.worldPosition = mul(position, gTransformationMatrix.World).xyz;
    
	return output;
}