Texture2D composite_renderer_terrain_tex : register(t0);
Texture2D composite_renderer_gui_tex : register(t1);

SamplerState sampler0 : register(s0);

struct PS_INPUT
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 color = composite_renderer_gui_tex.Sample(sampler0, input.uv);
    if (color.r == 0 && color.g == 0 && color.b == 0)
        color = composite_renderer_terrain_tex.Sample(sampler0, input.uv);
    color.w = 1;
	return color;
}