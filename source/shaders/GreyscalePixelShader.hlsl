
//Define 2D Texture and Sampler State
Texture2D <float4> g_accessibilityTexture;

SamplerState g_samLinear;

int simulationType : register(c0);

//Define Input type : NORMAL, TEXCOORD0 means from vertex shader
struct PS_INPUT
{
    float3 vNormal		: NORMAL;
    float2 vTexcoord	: TEXCOORD0;
};


float3 greyscaleFilter(float3 color) {
    color = saturate(color);

    float r_greyscale = color.r * 0.299 + color.g * 0.587 + color.b * 0.114;
    float g_greyscale = color.r * 0.299 + color.g * 0.587 + color.b * 0.114;
    float b_greyscale = color.r * 0.299 + color.g * 0.587 + color.b * 0.114;

    r_greyscale = saturate(r_greyscale);
    g_greyscale = saturate(g_greyscale);
    b_greyscale = saturate(b_greyscale);
    return float3(r_greyscale, g_greyscale, b_greyscale);
}

float3 greyscaleMain(PS_INPUT Input) : SV_TARGET
{
    float3 color = g_accessibilityTexture.Sample(g_samLinear, Input.vTexcoord);
    return greyscaleFilter(color);
}
