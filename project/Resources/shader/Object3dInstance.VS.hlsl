#include "Object3d.hlsli"

struct Object3dInstanceForGPU
{
    float4x4 World;
    float4x4 WVP;
    float4x4 WorldInverseTranspose;
};
StructuredBuffer<Object3dInstanceForGPU> gInstance : register(t0);

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input, uint instanceID : SV_InstanceID)
{
    VertexShaderOutput output;
    output.position = mul(input.position, gInstance[instanceID].WVP);
    output.texcoord = input.texcoord;
    //法線変換
    float3 worldNormal = mul(input.normal, (float3x3) gInstance[instanceID].World);
    output.normal = normalize(worldNormal);
    
    //カメラのワールド座標を渡す
    output.worldPosition = mul(input.position, gInstance[instanceID].World).xyz;
    return output;
}