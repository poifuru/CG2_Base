#include "Fullscreen.hlsli"

struct OutlineBuffer
{
    float4 color; // アウトラインの色 (RGB) と不透明度 (A)
    float edgeThreshold; // エッジと判定する閾値 (小さいほど敏感になる)
    float edgeThickness; // エッジの太さ (ピクセル単位の倍率。通常 1.0)
    float cameraNear; // カメラのNear面
    float cameraFar; // カメラのFar面
    float3 padding; // アライメント用
};

ConstantBuffer<OutlineBuffer> gOutlineBuffer : register(b0);
Texture2D<float4> gTexture : register(t0, space2); // 描画済みのメイン画像
Texture2D<float> gDepthTexture : register(t1, space2); // デプスバッファ (R24_UNORM_X8_TYPELESSなど)
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET;
};

// 深度値(0.0〜1.0)から、ビュー空間の実距離(Linear Depth)を復元する関数
float GetLinearDepth(float2 uv, float near, float far)
{
    float depth = gDepthTexture.Sample(gSampler, uv).r;
    // 遠近による非線形な深度値を線形(カメラからの距離)に変換する
    return (near * far) / (far - depth * (far - near));
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    // 現在のピクセルの元の色
    float4 sceneColor = gTexture.Sample(gSampler, input.texcoord);
    
    // テクスチャの解像度を取得して、1ピクセルあたりのUVサイズを計算
    float width, height;
    gDepthTexture.GetDimensions(width, height);
    float2 texelSize = float2(1.0f / width, 1.0f / height) * gOutlineBuffer.edgeThickness;
    float near = gOutlineBuffer.cameraNear;
    float far = gOutlineBuffer.cameraFar;
    float2 uv = input.texcoord;
    
    // --- ラプラシアンフィルタによるエッジ検出 ---
    // 中心ピクセルと、上下左右の4つの隣接ピクセルの線形深度を取得
    float center = GetLinearDepth(uv, near, far);
    float up = GetLinearDepth(uv + float2(0.0f, -1.0f) * texelSize, near, far);
    float down = GetLinearDepth(uv + float2(0.0f, 1.0f) * texelSize, near, far);
    float left = GetLinearDepth(uv + float2(-1.0f, 0.0f) * texelSize, near, far);
    float right = GetLinearDepth(uv + float2(1.0f, 0.0f) * texelSize, near, far);
    
    // ラプラシアン（2階微分）を計算して、深度の急激な変化（エッジ）を検出
    float laplacian = abs(up + down + left + right - 4.0f * center);
    
    // 【重要】遠くのオブジェクトほどデプスの絶対値の差が大きくなりやすいので、
    // 中心ピクセルの距離で割ることで、遠近に関わらず一定の太さでエッジが出るように「正規化」する
    float edgeDepth = laplacian / center;
    
    // 閾値を超えたらエッジとする (smoothstepでエッジの境界をほんの少し滑らかにする)
    float edge = smoothstep(gOutlineBuffer.edgeThreshold, gOutlineBuffer.edgeThreshold + 0.005f, edgeDepth);
    
    // 元のカラーとアウトラインカラーをブレンドする
    float3 finalColor = lerp(sceneColor.rgb, gOutlineBuffer.color.rgb, edge * gOutlineBuffer.color.a);
    output.color = float4(finalColor, sceneColor.a);
    return output;
}