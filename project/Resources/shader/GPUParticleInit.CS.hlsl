struct DrawArguments {
    uint IndexCountPerInstance;
    uint InstanceCount;
    uint StartIndexLocation;
    int  BaseVertexLocation;
    uint StartInstanceLocation;
};
RWStructuredBuffer<uint> gFreeList : register(u0);
RWStructuredBuffer<int> gFreeListCounter : register(u1);
RWStructuredBuffer<DrawArguments> gDrawArguments : register(u2);

cbuffer InitParams : register(b0) {
    uint gIndexCount;
};

[numthreads(256, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
    if (DTid.x >= 10000) return;
    
    gFreeList[DTid.x] = DTid.x;
    
    if (DTid.x == 0) {
        gFreeListCounter[0] = 10000;
        
        gDrawArguments[0].IndexCountPerInstance = gIndexCount;
        gDrawArguments[0].InstanceCount = 0;
        gDrawArguments[0].StartIndexLocation = 0;
        gDrawArguments[0].BaseVertexLocation = 0;
        gDrawArguments[0].StartInstanceLocation = 0;
    }
}
