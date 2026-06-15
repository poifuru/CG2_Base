#include "Fullscreen.hlsli"

struct FogBuffer
{
    matrix inverseVP;   // 64バイト
    float4 color;       // 16バイト
    float start;        // 4バイト
    float end;          // 4バイト
    float cameraNear;   // 4バイト
    float cameraFar;    // 4バイト
    float heightStart;  // 4バイト
    float heightEnd;    // 4バイト
    float2 padding;     // 8バイト
};

ConstantBuffer<FogBuffer> gFogBuffer : register(b0);
Texture2D<float4> gTexture : register(t0); // 描画済みのシーンカラー
Texture2D<float> gDepthTexture : register(t1); // 深度バッファ (R32_FLOATなど)
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    // 現在のピクセルの色と深度値をサンプリング
    float4 sceneColor = gTexture.Sample(gSampler, input.texcoord);
    float depthAttr = gDepthTexture.Sample(gSampler, input.texcoord);
    
    // --- ワールド座標の復元 ---
    float x = input.texcoord.x * 2.0f - 1.0f;
    float y = (1.0f - input.texcoord.y) * 2.0f - 1.0f;
    float z = depthAttr;
    float4 ndcPos = float4(x, y, z, 1.0f);
    
    float4 worldPos = mul(ndcPos, gFogBuffer.inverseVP);
    worldPos /= worldPos.w; // ワールド座標
    
    // カメラの値を扱いやすく
    float near = gFogBuffer.cameraNear;
    float far = gFogBuffer.cameraFar;
    
    // 深度バッファの値を実際の距離(ビュー空間のZ)に復元
    float viewZ = (near * far) / (far - depthAttr * (far - near));
    
    // 線形フォグの係数(f)を計算 (0.0〜1.0)
    float fogFactor = saturate((gFogBuffer.end - viewZ) / (gFogBuffer.end - gFogBuffer.start));
    
    // 高さフォグの係数 (0.0: 深い 〜 1.0: 浅い)
    float heightFactor = saturate((worldPos.y - gFogBuffer.heightEnd) / (gFogBuffer.heightStart - gFogBuffer.heightEnd));
    
    // 奥行と高さのフォグを合成 (いずれか一方が濃ければ濃くなる)
    float combinedFactor = fogFactor * heightFactor;
    float finalBlend = (1.0f - combinedFactor) * gFogBuffer.color.a;
    
    // 深いほどフォグ色自体を暗くする (最大90%暗く)
    float3 fogColor = gFogBuffer.color.rgb * lerp(0.1f, 1.0f, heightFactor);
    
    // フォグの色とシーンの色を補間
    float3 finalColor = lerp(sceneColor.rgb, fogColor, finalBlend);
    
    // 代入
    output.color.rgb = finalColor;
    output.color.a = sceneColor.a;
    
    return output;
}