#include "GPUParticle.hlsli"

RWStructuredBuffer<DrawArguments> gDrawArguments : register(u0);

[numthreads(1, 1, 1)]
void main() {
    gDrawArguments[0].InstanceCount = 0;
}
