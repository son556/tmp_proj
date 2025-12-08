Texture2D water_init_depth_tex : register(t0);

SamplerState sampler0 : register(s0);

struct PS_INPUT
{
    float4 pos : SV_Position;
    float4 c_pos : CLIP_SPACE;
    float4 w_pos : WORLD_SPACE;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct PS_OUTPUT
{
    float4 w_pos : SV_Target0;
    float4 color : SV_Target1;
};

float collisionPlane(float3 o, float3 d, float3 n);

PS_OUTPUT main(PS_INPUT input)
{
    PS_OUTPUT output;
    float2 uv;
    float3 ndc = input.c_pos.xyz / input.c_pos.w;
    uv.x = (ndc.x + 1.0f) * 0.5f;
    uv.y = (-ndc.y + 1.0f) * 0.5f;
    float d = water_init_depth_tex.Sample(sampler0, uv).r;
    if (ndc.z >= d)
        discard;
    
    output.color = float4(0, 0, 1, 1);
    output.w_pos = input.w_pos;
    return output;
}