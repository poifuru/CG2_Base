#include "GPUParticle.hlsli"

RWStructuredBuffer<GPUParticle> gParticles : register(u0);
RWStructuredBuffer<uint> gFreeList : register(u1);
RWStructuredBuffer<int> gFreeListCounter : register(u2);

StructuredBuffer<ParticleEmitRequest> gEmitRequests : register(t0);

cbuffer EmitParams : register(b0) {
    uint gEmitCount;
};

[numthreads(256, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
    if (DTid.x >= gEmitCount) return;

    // 空きスロットインデックスを取得
    int counterVal;
    InterlockedAdd(gFreeListCounter[0], -1, counterVal);

    if (counterVal > 0) {
        uint particleIdx = gFreeList[counterVal - 1];

        // 該当の要求情報を取り出す
        ParticleEmitRequest req = gEmitRequests[DTid.x];

        // 初期パラメータ設定
        gParticles[particleIdx].position = req.position;
        gParticles[particleIdx].velocity = req.velocity;
        gParticles[particleIdx].color = req.color;
        gParticles[particleIdx].maxLifeTime = req.lifeTime;
        gParticles[particleIdx].currentTime = 0.0f;
        gParticles[particleIdx].scale = req.scale;
        gParticles[particleIdx].rotate = req.rotate;
        gParticles[particleIdx].active = 1;
    } else {
        // カウンタが空なら元に戻す
        InterlockedAdd(gFreeListCounter[0], 1);
    }
}
