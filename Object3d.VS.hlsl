#include "Object3d.hlsli"

struct TransformaitionMatrix
{
    float32_t4x4 WVP;
    float32_t4x4 World;
};
ConstantBuffer<TransformaitionMatrix> gTransformaitionMatrix : register(b0);

struct VertexShaderInput
{
    float32_t4 position : POSITION0;
    float32_t2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.position = mul(input.position, gTransformaitionMatrix.WVP);
    output.texcoord = input.texcoord;
    //output.normal = normalize(mul(input.normal, (float32_t3x3) gTransformaitionMatrix.World));//これがエラーの原因ぽい
    output.normal = normalize(mul(input.normal, (float3x3) gTransformaitionMatrix.World));
    return output;
}   