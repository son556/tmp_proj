Texture2D gui_texture : register(t0);

SamplerState sampler0 : register(s0);


struct PS_INPUT
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD;
};

cbuffer flag : register(b0)
{
    bool x_flag;
}

float4 main(PS_INPUT input) : SV_Target
{
    float4 color;
    color = gui_texture.Sample(sampler0, input.uv);
    color.rgb = pow(color.rgb, 1.0 / 2.2);
    if (x_flag && (color.r == 0 || color.g))
        discard;
    color.w = 1;
    return color;
}