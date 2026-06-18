struct VS_Input
{
    float4 pos : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

struct PS_Input
{
    float4 sv_pos : SV_POSITION;
    float2 uv : TEXCOORD;
};