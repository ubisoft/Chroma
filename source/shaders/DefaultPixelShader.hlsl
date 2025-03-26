
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

float3 defaultMain(PS_INPUT Input) : SV_TARGET
{
    float3 color = g_accessibilityTexture.Sample(g_samLinear, Input.vTexcoord);
    return color;
}