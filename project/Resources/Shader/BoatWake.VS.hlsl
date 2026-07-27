struct TransformationMatrix
{
    float4x4 WVP;
    float4x4 World;
    float4x4 WorldInverseTranspose;
};

struct VertexShaderInput
{
    float3 position : POSITION0;
    float alpha : ALPHA0;
    float2 uv : UV0;
};

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float alpha : TEXCOORD1;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    
    // 水面との重なり（Zファイト/チラつき）を防ぐためY座標を少し（3cmほど）浮かせる
    float3 worldPos = input.position;
    worldPos.y += 0.03f;
    
    // WVP行列を掛けてクリップ座標系に変換
    output.position = mul(float4(worldPos, 1.0f), gTransformationMatrix.WVP);
    
    // UV座標とアルファ値をそのままピクセルシェーダーへ渡す
    output.uv = input.uv;
    output.alpha = input.alpha;
    return output;
}