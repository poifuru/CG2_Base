#include "Skybox.hlsli"

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

struct Material
{
    float4 color;
    float4x4 uvTransform;
    float roughness; // 粗さ
    float metallic; // 金属度
    float environmentCoefficient; // 環境係数
    int enableLighting;
};

ConstantBuffer<Material> gMaterial : register(b0);

TextureCube<float4> gTexture : register(t0);

SamplerState gSampler : register(s0);

PixelShaderOutput main(VertexShaderOutput input){
	PixelShaderOutput output;
	float4 textureColor = gTexture.Sample(gSampler, input.texcoord);
	output.color = textureColor * gMaterial.color;
	return output;
}