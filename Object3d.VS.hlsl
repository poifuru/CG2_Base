struct TransformaitionMatrix
{
    float32_t4x4 WVP;
};
ConstantBuffer<TransformaitionMatrix> gTransformaitionMatrix : register(b0);

struct VertexShaderOutput
{
    float32_t4 position : SV_POSITION;
};

struct VertexShaderInput
{
    float32_t4 position : POSITION0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.position = mul(input.position, gTransformaitionMatrix.WVP);
    return output;
}