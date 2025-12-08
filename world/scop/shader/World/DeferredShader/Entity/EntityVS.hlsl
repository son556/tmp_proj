struct VS_INPUT
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
    float3 n : NORMAL;
};

struct PS_INPUT
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD;
    float4 w_pos : WORLD_POSITION;
    float4 w_normal : WORLD_NORMAL;
};

cbuffer mvp : register(b0)
{
    matrix world;
    matrix view;
    matrix proj;
}

PS_INPUT main( VS_INPUT input )
{
    PS_INPUT output;
    
    output.pos = float4(input.pos, 1);
    output.pos = mul(output.pos, world);
    output.w_pos = output.pos;
    output.w_normal = mul(float4(input.n, 0), world);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, proj);
    output.uv = input.uv;
    return output;
}