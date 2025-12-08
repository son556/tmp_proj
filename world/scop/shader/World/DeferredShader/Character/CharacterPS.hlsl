struct PS_INPUT
{
    float4 pos : SV_Position;
    float4 world_pos : WORLD_POSITION;
    float4 view_pos : VIEW_POSITION;
    float4 clip_pos : CLIP_POSITION;
    float2 uv : TEXCOORD;
};

Texture2D charcater_color_tex : register(t0);
Texture2D tex_depth : register(t1);
SamplerState sampler_0 : register(s0);
SamplerState sampler_1 : register(s1);

cbuffer constant_buffer : register(b0)
{
    float2 flag_WaterHeight;
};

struct PS_OUTPUT
{
    float4 accum : SV_Target0;
    float reveal : SV_Target1;
};

float4 accum_0(PS_INPUT input, float4 color)
{
    // weight function
    float z_near = 0.01;
    float z_far = 1000;
    float z = z_near * z_far * (input.view_pos.z - z_far) / (z_near - z_far);
    float weight =
        clamp(pow(min(1.0, color.a * 10.0) + 0.01, 3.0) * 1e8 *
            pow(1.0 - z * 0.9, 3.0), 0.01, 3000);
    
    float4 res =
        float4(color.rgb * color.a, color.a) * weight;
    return res;
}

PS_OUTPUT main(PS_INPUT input)
{
    PS_OUTPUT output;
    
    if (flag_WaterHeight.x && input.world_pos.y < flag_WaterHeight.y)
        discard;
    else if (flag_WaterHeight.x == 0 && input.world_pos.y >= flag_WaterHeight.y)
        discard;
    
    float2 texcoord;
    float3 ndc_p = input.clip_pos.xyz / input.clip_pos.w;
    texcoord.x = (ndc_p.x + 1.0f) * 0.5f;
    texcoord.y = -1.0f * (ndc_p.y + 1.0f) * 0.5f;
    float std_z = tex_depth.Sample(sampler_1, texcoord).r;
    if (ndc_p.z > std_z + 0.00001)
        discard;
    float4 color = charcater_color_tex.Sample(sampler_0, input.uv);
    color.a = 0.42;
    
    output.accum = accum_0(input, color);
    output.reveal = color.a;
    
	return output;
}