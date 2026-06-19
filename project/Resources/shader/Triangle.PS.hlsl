#include "Triangle.hlsli"

// マテリアルの構造体の定義
struct MaterialData
{
    float4 color;
    float4x4 uvTransform;
    float roughness;
    float metallic;
    float environmentCoefficient;
    int enableLighting;
};

// 【Slot 1】から送られてくる、今回のオブジェクトインデックス
struct PushIndices
{
    uint materialIndex;
    uint textureIndex;
};

// 【Slot 2 (space1)】：すべてのリソースが同居する巨大な本棚空間
// 構造化バッファの配列（t0, space1）
StructuredBuffer<MaterialData> g_BindlessBuffers[] : register(t0, space1);
// テクスチャの配列（t0, space2）※スペース2に分けてバインドレスで読み込む
Texture2D g_Textures[] : register(t0, space2);

// 静的サンプラー (s0, space0)
SamplerState g_Sampler : register(s0, space0);

// 【Slot 1 (space0)】：CPUからドローコールごとに届くインデックス
ConstantBuffer<PushIndices> g_Indices : register(b1, space0);

float4 main(PS_Input input) : SV_Target
{
    // 1. まず、CPUから送られてきたマテリアル自体のインデックスを取得
    uint matIdx = g_Indices.materialIndex;
    
    // 2. マテリアル配列から、自分のマテリアルデータを引き抜く
    MaterialData myMaterial = g_BindlessBuffers[matIdx][0];
    
    // 3. プッシュ定数から「テクスチャ番号」を取り出す！
    uint texIdx = g_Indices.textureIndex;
    
    // 4. テクスチャ配列から、その番号の画像をダイレクトにサンプリング！
    float4 texColor = g_Textures[texIdx].Sample(g_Sampler, input.uv);
    
    // 5. テクスチャの色とマテリアル色を掛け合わせて出力（今回は白なので画像がそのまま出る）
    return texColor * myMaterial.color;
}