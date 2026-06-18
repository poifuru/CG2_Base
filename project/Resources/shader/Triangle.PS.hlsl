#include "Triangle.hlsli"

// マテリアルの構造体の定義
struct MaterialData
{
    float4 color;
    uint textureIndex;
    uint padding[3];
};

// 【Slot 2】にステージングされている、ディスクリプタヒープ全体を指す無限配列 (space1, t0)
// この中に、構造化バッファのSRV案内札も混ざって並んでいる！
StructuredBuffer<MaterialData> g_BindlessBuffers[] : register(t0, space1);

// 【Slot 1】から送られてくる、今回のオブジェクトインデックス
struct PushIndices
{
    uint materialIndex;
    uint textureIndex;
};
ConstantBuffer<PushIndices> g_Indices : register(b1, space0);

float4 main(PS_Input input) : SV_Target
{
    // 1. CPUから Slot 1 で送られてきた「マテリアルバッファのインデックス」を取得
    uint matIdx = g_Indices.materialIndex;
    
    // 2. 巨大配列から、自分のマテリアルの構造化バッファをダイレクトに狙い撃ち！
    // 構造化バッファの 0 番目の要素に色データが入っている
    MaterialData myMaterial = g_BindlessBuffers[matIdx][0];
    
    // 3. 焼き込まれた色をそのまま出力
    return myMaterial.color;
}