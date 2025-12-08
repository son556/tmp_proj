struct VS_INPUT
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
};

struct PS_INPUT
{
    float4 ndc_pos : SV_Position;
    float3 world_pos : POSITION;
    float2 uv : TEXCOORD;
};

cbuffer inv_view_proj : register(b0)
{
    float4x4 inv_vp;
    float4x4 world;
}


PS_INPUT main( VS_INPUT input )
{
    PS_INPUT output;
    output.ndc_pos = float4(input.pos, 1);
    output.ndc_pos = mul(output.ndc_pos, world);
    output.world_pos = mul(output.ndc_pos, inv_vp);
    output.uv = input.uv;
	return output;
}