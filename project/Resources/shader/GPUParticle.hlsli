struct GPUParticle {
    float3 position;
    float3 velocity;
    float4 color;
    float maxLifeTime;
    float currentTime;
    float3 scale;
    float3 rotate;
    uint active;
};

struct ParticleForGPU {
    float4x4 World;
    float4x4 WVP;
    float4 color;
};

struct DrawArguments {
    uint IndexCountPerInstance;
    uint InstanceCount;
    uint StartIndexLocation;
    int  BaseVertexLocation;
    uint StartInstanceLocation;
};

struct ParticleEmitRequest {
    float3 position;
    float3 velocity;
    float4 color;
    float lifeTime;
    float3 scale;
    float3 rotate;
};
