struct VS_INPUT
{
    int tex_arr_idx : INDEX;
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 uv : TEXCOORD;
};

cbuffer MVP : register(b0)
{
    matrix world;
    matrix view;
    matrix proj;
};

struct PS_INPUT
{
    int tex_arr_idx : INDEX;
    float4 pos : SV_Position;
    float3 w_pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 uv : TEXCOORD;
};


PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
    output.pos = float4(input.pos, 1);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, proj);
    
    output.w_pos = input.pos;
    output.normal = input.normal;
    
    output.uv = input.uv;
    output.tangent = input.tangent;
    output.tex_arr_idx = input.tex_arr_idx;
    return output;
}