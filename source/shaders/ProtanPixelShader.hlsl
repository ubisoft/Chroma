
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

float3 rgb2lin(float3 c) { return (0.992052 * pow(c, 2.2) + 0.003974) * 128.498039; }
float3 lin2rgb(float3 c) { return pow(c, 0.45454545); }



float3 protanDeutanFilter(int k1, int k2, int k3, float3 color) {
    color = saturate(color);
    float3 c_lin = rgb2lin(color);

    float r_blind = (k1 * c_lin.r + k2 * c_lin.g) / 16448.25098;
    float b_blind = (k3 * c_lin.r - k3 * c_lin.g + 128.498039 * c_lin.b) / 16448.25098;
    r_blind = saturate(r_blind);
    b_blind = saturate(b_blind);

    return lerp(color, lin2rgb(float3(r_blind, r_blind, b_blind)), 1.0);
}

float3 protanMain(PS_INPUT Input) : SV_TARGET
{
    float3 color = g_accessibilityTexture.Sample(g_samLinear, Input.vTexcoord);
	return protanDeutanFilter(14.443137, 114.054902, 0.513725, color);
}