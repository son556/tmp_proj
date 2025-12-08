struct PS_INPUT
{
    float4 pos : SV_Position;
    float4 ndc_pos : NDC;
};

float4 main(PS_INPUT input) : SV_Target
{
    input.ndc_pos /= input.ndc_pos.w;
    return float4(1.0 - input.ndc_pos.z, 0, 0, 1);
}