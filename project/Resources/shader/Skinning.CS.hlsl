struct Well
{
    float4x4 skeletonSpaceMatrix;
    float4x4 skeletonSpaceInverseTransposeMatrix;
};

struct Vertex
{
    float4 position;
    float2 texcoord;
    float3 normal;
};

struct VertexInfluence
{
    float4 weight;
    int4 index;
};

struct SkinningInformation
{
    uint numVertices;
};

StructuredBuffer<Well> gMatrixPalette : register(t0);
StructuredBuffer<Vertex> gInputVertices : register(t1);
StructuredBuffer<VertexInfluence> gInfluence : register(t2);
RWStructuredBuffer<Vertex> gOutputVertices : register(u0);
ConstantBuffer<SkinningInformation> gSkinningInformation : register(b0);

// Skinningを行う関数
Vertex Skinning(Vertex input, VertexInfluence influence)
{
    Vertex skinned;
    
    // 最初に0で完全に初期化
    skinned.position = (float4) 0;
    skinned.normal = (float3) 0;
    
    //*** Skinningの処理***//
    // 位置の変換
    skinned.position += mul(input.position, gMatrixPalette[influence.index.x].skeletonSpaceMatrix) * influence.weight.x;
    skinned.position += mul(input.position, gMatrixPalette[influence.index.y].skeletonSpaceMatrix) * influence.weight.y;
    skinned.position += mul(input.position, gMatrixPalette[influence.index.z].skeletonSpaceMatrix) * influence.weight.z;
    skinned.position += mul(input.position, gMatrixPalette[influence.index.w].skeletonSpaceMatrix) * influence.weight.w;
    skinned.position.w = 1.0f; // 確実に1を入れる
    
    // 法線の変換
    skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[influence.index.x].skeletonSpaceInverseTransposeMatrix) * influence.weight.x;
    skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[influence.index.y].skeletonSpaceInverseTransposeMatrix) * influence.weight.y;
    skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[influence.index.z].skeletonSpaceInverseTransposeMatrix) * influence.weight.z;
    skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[influence.index.w].skeletonSpaceInverseTransposeMatrix) * influence.weight.w;
    skinned.normal = normalize(skinned.normal); // 正規化して戻してあげる
    
    return skinned;
}

[numthreads(1024, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    // 処理したい頂点数以降のスレッドには何もさせない
    uint vertexIndex = DTid.x;
    if (vertexIndex < gSkinningInformation.numVertices)
    {
        // Skinning計算
        // 必要なデータをStructuredBufferからとってくる
        Vertex input = gInputVertices[vertexIndex];
        VertexInfluence influence = gInfluence[vertexIndex];
        
        // Skinning後の頂点を計算
        Vertex skinned = Skinning(input, influence);
        skinned.texcoord = input.texcoord; // ★Skinning呼び出しの後にコピーする
        
        // Skinning後の頂点データを書き込む
        gOutputVertices[vertexIndex] = skinned;
    }
}