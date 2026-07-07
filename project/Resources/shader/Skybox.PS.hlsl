#include "Skybox.hlsli"

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

struct MaterialData
{
    float4 color;
    float4x4 uvTransform;
    float roughness; // 粗さ
    float metallic; // 金属度
    float environmentCoefficient;	// 環境係数
    int enableLighting;
};

// 【Slot 1 (space0)】: インデックス定数
struct PushIndices
{
    uint materialIndex;
    uint textureIndex;
};
ConstantBuffer<PushIndices> g_Indices : register(b1, space0);

// 【Slot 2 (space1)】: 巨大SRVヒープ (マテリアルデータ)
StructuredBuffer<MaterialData> g_BindlessBuffers[] : register(t0, space1);

// 【Slot 3 (space2)】: 巨大SRVヒープ (テクスチャ)
TextureCube<float4> g_TexturesCube[] : register(t0, space2);

// サンプラー (s0, space0)
SamplerState gSampler : register(s0, space0);

PixelShaderOutput main(VertexShaderOutput input){
	PixelShaderOutput output;
	
	// インデックスを使ってバインドレスバッファからマテリアルとテクスチャを取得
	MaterialData material = g_BindlessBuffers[g_Indices.materialIndex][0];
	float4 textureColor = g_TexturesCube[g_Indices.textureIndex].Sample(gSampler, input.texcoord);
	
	output.color = textureColor * material.color;
	return output;
}