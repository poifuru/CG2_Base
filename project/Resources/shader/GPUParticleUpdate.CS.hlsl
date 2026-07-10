#include "GPUParticle.hlsli"

struct UpdateParams {
    float4x4 cameraWorld;
    float4x4 vp;
    float deltaTime;
    uint useBillboard;
};

RWStructuredBuffer<GPUParticle> gParticles : register(u0);
RWStructuredBuffer<uint> gFreeList : register(u1);
RWStructuredBuffer<int> gFreeListCounter : register(u2);
RWStructuredBuffer<ParticleForGPU> gDrawParticles : register(u3);
RWStructuredBuffer<DrawArguments> gDrawArguments : register(u4);

ConstantBuffer<UpdateParams> gUpdateParams : register(b0);

// アフィン行列作成ヘルパー
float4x4 MakeAffineMatrix(float3 scale, float3 rotate, float3 translate) {
    float3 s = sin(rotate);
    float3 c = cos(rotate);

    float4x4 rx = float4x4(
        1, 0, 0, 0,
        0, c.x, s.x, 0,
        0, -s.x, c.x, 0,
        0, 0, 0, 1
    );

    float4x4 ry = float4x4(
        c.y, 0, -s.y, 0,
        0, 1, 0, 0,
        s.y, 0, c.y, 0,
        0, 0, 0, 1
    );

    float4x4 rz = float4x4(
        c.z, s.z, 0, 0,
        -s.z, c.z, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );

    float4x4 r = mul(mul(rz, rx), ry);

    float4x4 m = float4x4(
        scale.x * r._m00, scale.x * r._m01, scale.x * r._m02, 0,
        scale.y * r._m10, scale.y * r._m11, scale.y * r._m12, 0,
        scale.z * r._m20, scale.z * r._m21, scale.z * r._m22, 0,
        translate.x, translate.y, translate.z, 1
    );
    return m;
}

[numthreads(256, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
    uint particleIdx = DTid.x;
    uint maxParticles = 10000;
    if (particleIdx >= maxParticles) return;

    if (gParticles[particleIdx].active == 1) {
        // 物理計算 (等速運動)
        gParticles[particleIdx].position += gParticles[particleIdx].velocity * gUpdateParams.deltaTime;
        gParticles[particleIdx].currentTime += gUpdateParams.deltaTime;

        if (gParticles[particleIdx].currentTime >= gParticles[particleIdx].maxLifeTime) {
            // 死亡
            gParticles[particleIdx].active = 0;

            // フリーリストに戻す
            int counterVal;
            InterlockedAdd(gFreeListCounter[0], 1, counterVal);
            if (counterVal >= 0 && counterVal < (int)maxParticles) {
                gFreeList[counterVal] = particleIdx;
            }
        } else {
            // 生存
            // 描画データバッファのインデックスをアトミックに取得
            int drawIdx;
            InterlockedAdd(gDrawArguments[0].InstanceCount, 1, drawIdx);

            if (drawIdx < (int)maxParticles) {
                // スケールと回転を適用した行列の作成（平行移動は一旦原点）
                float4x4 world = MakeAffineMatrix(gParticles[particleIdx].scale, gParticles[particleIdx].rotate, float3(0.0f, 0.0f, 0.0f));

                if (gUpdateParams.useBillboard != 0) {
                    float4x4 billboardRotation = gUpdateParams.cameraWorld;
                    billboardRotation._m30 = 0.0f;
                    billboardRotation._m31 = 0.0f;
                    billboardRotation._m32 = 0.0f;
                    billboardRotation._m33 = 1.0f;
                    world = mul(world, billboardRotation);
                }

                // 最後に位置を平行移動として適用
                world._m30 = gParticles[particleIdx].position.x;
                world._m31 = gParticles[particleIdx].position.y;
                world._m32 = gParticles[particleIdx].position.z;

                gDrawParticles[drawIdx].World = world;
                gDrawParticles[drawIdx].WVP = mul(world, gUpdateParams.vp);

                // アルファフェードアウトの適用
                gDrawParticles[drawIdx].color = gParticles[particleIdx].color;
                float alpha = 1.0f - (gParticles[particleIdx].currentTime / gParticles[particleIdx].maxLifeTime);
                gDrawParticles[drawIdx].color.w = alpha;
            }
        }
    }
}
