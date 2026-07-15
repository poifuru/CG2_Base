#pragma once

struct MaterialData
{
    float4 color;
    float4x4 uvTransform;
    float roughness; // 粗さ
    float metallic; // 金属度
    float environmentCoefficient; // 環境係数
    int enableLighting;
    float time;
};