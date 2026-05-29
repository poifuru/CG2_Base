#include "CopyImage.hlsli"

struct Vertex {
    float4 pos;
    float2 uv;
};

// データを構造体でまとめて、順番を確実に固定する
static const Vertex kVertices[3] = {
    { float4(-1.0f,  1.0f, 0.0f, 1.0f), float2(0.0f, 0.0f) }, // 左上
    { float4( 3.0f,  1.0f, 0.0f, 1.0f), float2(2.0f, 0.0f) }, // 右上
    { float4(-1.0f, -3.0f, 0.0f, 1.0f), float2(0.0f, 2.0f) }  // 左下
};

VertexShaderOutput main(uint vertexId : SV_VertexID) {
    VertexShaderOutput output;
    output.position = kVertices[vertexId].pos;
    output.texcoord = kVertices[vertexId].uv;
    return output;
}