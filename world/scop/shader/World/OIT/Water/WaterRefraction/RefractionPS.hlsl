Texture2D color_tex_refraction : register(t0);
Texture2D distortion_tex_refraction : register(t1);
SamplerState sampler0 : register(s0);

struct PS_INPUT
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD;
};

cbuffer constant_buffer : register(b0)
{
    float move_factor;
}

static float wave_strength = 0.0132;

float4 main(PS_INPUT input) : SV_TARGET
{
    float2 uv = input.uv;
    uv.y *= -1;
    uv.x += move_factor;
    float offset1 = distortion_tex_refraction.Sample(sampler0, uv).rg;
    offset1 = (offset1 * 2 - float2(1, 1)) * wave_strength;
    
    uv.x = -input.uv.x;
    uv.y += move_factor;
    float offset2 = distortion_tex_refraction.Sample(sampler0, uv).rg;
    offset2 = (offset2 * 2 - float2(1, 1)) * wave_strength;
    
    float offset = offset1 + offset2;
    input.uv += offset;
    input.uv.x = clamp(input.uv.x, 0, 1);
    input.uv.y = clamp(input.uv.y, 0.01, 1);
    float3 color = color_tex_refraction.Sample(sampler0, input.uv).rgb;
    return float4(color, 1);
}