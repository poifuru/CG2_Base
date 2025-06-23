#include "Object3d.hlsli"

struct TransformaitionMatrix
{
    float32_t4x4 WVP;
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
    return output;
}