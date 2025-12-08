Texture2D gui_texture : register(t0);
Texture2DArray block_color : register(t1);

SamplerState sampler0 : register(s0);


struct PS_INPUT
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD;
};

cbuffer array_idx : register(b0)
{
    float4 idx;
}

float4 main(PS_INPUT input) : SV_Target
{
    float4 color;
    if (idx.x == -1)
        color = gui_texture.Sample(sampler0, input.uv);
    else
        color = block_color.Sample(sampler0, float3(input.uv, (int) idx.x));
    if (idx.x != 8)
        color.rgb = pow(color.rgb, 1.0 / 2.2);
    color.w = 1;
    return color;
}