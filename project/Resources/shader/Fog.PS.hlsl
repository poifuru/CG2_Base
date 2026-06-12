#include "Fullscreen.hlsli"

struct FogBuffer
{
    float4 color;       // 色
    float start;        // フォグの開始距離。オブジェクトが薄れ始める
    float end;          // フォグが最大まで掛かる距離
    float cameraNear;   // カメラのNearクリップ面
    float cameraFar;    // カメラのFarクリップ面
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
    
    // ★追加：深度が 1.0（初期値＝オブジェクトが何も描画されていない背景）ならフォグをかけない
    // ※ 浮動小数点の誤差を考慮して、1.0 ではなく 0.99999f などの閾値で比較するとより安全
    if (depthAttr >= 0.99999f)
    {
        output.color = sceneColor;
        return output;
    }
    
    // カメラの値を扱いやすく
    float near = gFogBuffer.cameraNear;
    float far = gFogBuffer.cameraFar;
    
    // 深度バッファの値を実際の距離(ビュー空間のZ)に復元
    float viewZ = (near * far) / (far - depthAttr * (far - near));
    
    // 線形フォグの係数(f)を計算
    float fogFactor = saturate((gFogBuffer.end - viewZ) / (gFogBuffer.end - gFogBuffer.start));
    
    // フォグの色とシーンの色を補間
    float3 finalColor = lerp(gFogBuffer.color.rgb, sceneColor.rgb, fogFactor);
    
    // 代入
    output.color.rgb = finalColor;
    output.color.a = sceneColor.a;
    
    return output;
}