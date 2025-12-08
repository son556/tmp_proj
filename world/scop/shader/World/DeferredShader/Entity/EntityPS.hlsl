struct PS_INPUT
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD;
    float4 w_pos : WORLD_POSITION;
    float4 w_normal : WORLD_NORMAL;
};

struct PS_OUTPUT
{
    float4 color : SV_Target0;
    float4 w_pos : SV_Target1;
    float4 w_normal : SV_Target2;
};

Texture2D entity_color_tex : register(t0);
SamplerState sampler_0 : register(s0);

PS_OUTPUT main(PS_INPUT input)
{
    PS_OUTPUT output;
    output.color = entity_color_tex.Sample(sampler_0, input.uv);
    output.w_pos = input.w_pos;
    output.w_normal = input.w_normal;
    return output;
}