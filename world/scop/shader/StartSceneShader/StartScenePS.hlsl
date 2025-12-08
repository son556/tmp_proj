struct PS_INPUT
{
    float4 ndc_pos : SV_Position;
    float3 world_pos : POSITION;
    float2 uv : TEXCOORD;
};

Texture2D start_scene_start_button_tex; // 0
Texture2D start_scene_start_button_select_tex; // 1
Texture2D start_scene_title_tex; // 2
Texture2D start_scene_letter_tex; // 3
TextureCube start_scene_sky_box; // 4

SamplerState sampler0 : register(s0);

cbuffer option : register(b0)
{
    float4 texture_idx;
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 color = float4(0, 0, 0, 1);
    if (texture_idx.x == 0)
    {
        color = start_scene_start_button_tex.Sample(sampler0, input.uv);
        color.rgb = pow(color.rgb, 1.0 / 2.2);
    }
    else if (texture_idx.x == 1)
    {
        color = start_scene_start_button_select_tex.Sample(sampler0, input.uv);
        if (color.r + color.g + color.b < 2.5)
            discard;
    }
    else if (texture_idx.x == 2)
    {
        color = start_scene_title_tex.Sample(sampler0, input.uv);
        if (color.r + color.g + color.b > 2.8)
            discard;
    }
    else if (texture_idx.x == 3)
    {
        color = start_scene_letter_tex.Sample(sampler0, input.uv);
        if (color.r + color.g + color.b < 0.3)
            discard;
    }
    else if (texture_idx.x == 4)
        color = start_scene_sky_box.Sample(sampler0, input.world_pos);
    color.w = 1;
	return color;
}