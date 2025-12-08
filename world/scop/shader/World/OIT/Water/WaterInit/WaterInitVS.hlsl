struct VS_INPUT
{
    float3 pos : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};


struct PS_INPUT
{
    float4 pos : SV_Position;
    float4 c_pos : CLIP_SPACE;
    float4 w_pos : WORLD_SPACE;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

cbuffer MVP : register(b0)
{
    matrix world;
    matrix view;
    matrix proj;
}

PS_INPUT main( VS_INPUT input )
{
    PS_INPUT output;
    output.pos = float4(input.pos, 1);
    output.w_pos = output.pos;
    output.pos = mul(output.pos, world);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, proj);
    output.c_pos = output.pos;
    output.color = input.color;
    output.uv = input.uv;
    return output;
}