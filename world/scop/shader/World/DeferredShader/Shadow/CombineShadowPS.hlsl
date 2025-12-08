struct MVP
{
    matrix world;
    matrix view;
    matrix proj;
};
StructuredBuffer<MVP> mvp_arr : register(t0);

Texture2D shadow_0 : register(t1);
Texture2D shadow_1 : register(t2);
Texture2D shadow_2 : register(t3);
Texture2D shadow_3 : register(t4);
Texture2D shadow_4 : register(t5);
Texture2D shadow_5 : register(t6);
Texture2D shadow_6 : register(t7);
Texture2D shadow_7 : register(t8);

Texture2D pos_tex : register(t9); // world space
Texture2D normal_tex : register(t10); // world space
Texture2D entity_pos_tex : register(t11); // world space
Texture2D entity_normal_tex : register(t12); // world space

SamplerState shadow_point_sampler
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = BORDER;
    AddressV = BORDER;
    AddressW = BORDER;
    BoarderColor = float4(100.f, 1.0f, 1.0f, 1.0f);
};

SamplerState sampler0
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
    AddressW = WRAP;
};

struct PS_INPUT
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD;
};

// 최대 8개 까지 나눔
cbuffer Split : register(b0)
{
    float4 light_dir; // world space
    float4 z_arr[8]; // view space
    matrix view;
};

float shadowCheck(float4 w_pos, int shadow_idx, float3 normal)
{   
    if (dot(float3(0, 1, 0), light_dir.xyz) < 0)
        return 0;
    w_pos += float4(normal, 0) * 0.1;
    w_pos = mul(w_pos, mvp_arr[shadow_idx].view);
    w_pos = mul(w_pos, mvp_arr[shadow_idx].proj);
    w_pos /= w_pos.w; // ndc
    float3 uvw = float3((w_pos.x + 1.0) * 0.5, (-w_pos.y + 1.0) * 0.5,
        shadow_idx); // ndc uv
    
    float z;
    
    if (shadow_idx == 0)
        z = shadow_0.Sample(shadow_point_sampler, uvw.xy).r;
    else if (shadow_idx == 1)
        z = shadow_1.Sample(shadow_point_sampler, uvw.xy).r;
    else if (shadow_idx == 2)
        z = shadow_2.Sample(shadow_point_sampler, uvw.xy).r;
    else if (shadow_idx == 3)
        z = shadow_3.Sample(shadow_point_sampler, uvw.xy).r;
    else if (shadow_idx == 4)
        z = shadow_4.Sample(shadow_point_sampler, uvw.xy).r;
    else if (shadow_idx == 5)
        z = shadow_5.Sample(shadow_point_sampler, uvw.xy).r;
    else if (shadow_idx == 6)
        z = shadow_6.Sample(shadow_point_sampler, uvw.xy).r;
    else
        z = shadow_7.Sample(shadow_point_sampler, uvw.xy).r;
    
    if (z < w_pos.z)
        return 0;
    return 1;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float res = -1;
    float4 w_pos = entity_pos_tex.Sample(sampler0, input.uv);
    if (w_pos.x == 0 && w_pos.y == 0 && w_pos.z == 0)
        w_pos = pos_tex.Sample(sampler0, input.uv);
    float4 p_eye = mul(w_pos, view);
    float3 normal = entity_normal_tex.Sample(sampler0, input.uv).xyz;
    if (normal.x == 0 && normal.y == 0 && normal.z == 0)
    {
        normal = normal_tex.Sample(sampler0, input.uv).xyz;
        if (normal.x == 0 && normal.y == 0 && normal.z == 0)
            discard;
    }
    
    [loop]
    for (int i = 0; i < light_dir.w; i++)
    {
        if (p_eye.z < z_arr[i].x)
        {
            if (p_eye.z > z_arr[i].x * 0.8 && p_eye.z < z_arr[i].x)
            {
                float r1 = 
                    shadowCheck(w_pos, i, normal);
                float r2 =
                    shadowCheck(w_pos, i + 1, normal);
                
                if (r1 == 0 || r2 == 0)
                    return float4(0, 0, 0, 1);
                else
                    return float4(1, 1, 1, 1);
            }
            else
            {
                res = shadowCheck(w_pos, i, normal);
                return float4(res, res, res, 1);
            }
        }
    }

    return float4(0, 0, 0, 0);
}