Texture2D water_reflection : register(t0);
Texture2D water_refraction : register(t1);
Texture2D terrain_tex : register(t2);
Texture2D water_pos_tex : register(t3);
SamplerState sampler0 : register(s0);

struct PS_INPUT
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD;
};

cbuffer cam_pos : register(b0)
{
    float4 cam_position;
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 color = water_reflection.Sample(sampler0, input.uv);
    if (color.r == 0 && color.g == 0 && color.b == 0)
        return terrain_tex.Sample(sampler0, input.uv);
    float4 ref_col = water_refraction.Sample(sampler0, input.uv);
    
    float3 cam_pos = cam_position.xyz;
    float3 water_pos = water_pos_tex.Sample(sampler0, input.uv).xyz;
    float3 normal = float3(0, cam_position.w, 0);
    float3 cam_vec = normalize(cam_pos - water_pos);
    float ndotv = saturate(dot(cam_vec, normal));
    float f = pow(ndotv, 0.5);
    
    color = lerp(color, ref_col, f);
    return color;
}