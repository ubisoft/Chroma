
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

float3 tritanFilter(float3 color) {
    color = saturate(color);

    float anchor_e0 = 0.05059983 + 0.08585369 + 0.00952420;
    float anchor_e1 = 0.01893033 + 0.08925308 + 0.01370054;
    float anchor_e2 = 0.00292202 + 0.00975732 + 0.07145979;
    float inflection = anchor_e1 / anchor_e0;

    float a1 = -anchor_e2 * 0.007009;
    float b1 = anchor_e2 * 0.0914;
    float c1 = anchor_e0 * 0.007009 - anchor_e1 * 0.0914;
    float a2 = anchor_e1 * 0.3636 - anchor_e2 * 0.2237;
    float b2 = anchor_e2 * 0.1284 - anchor_e0 * 0.3636;
    float c2 = anchor_e0 * 0.2237 - anchor_e1 * 0.1284;

    float3 c_lin = rgb2lin(color);

    float L = (c_lin.r * 0.05059983 + c_lin.g * 0.08585369 + c_lin.b * 0.00952420) / 128.498039;
    float M = (c_lin.r * 0.01893033 + c_lin.g * 0.08925308 + c_lin.b * 0.01370054) / 128.498039;
    float S = (c_lin.r * 0.00292202 + c_lin.g * 0.00975732 + c_lin.b * 0.07145979) / 128.498039;

    float tmp = M / L;

    if (tmp < inflection) S = -(a1 * L + b1 * M) / c1;
    else S = -(a2 * L + b2 * M) / c2;

    float r = L * 30.830854 - M * 29.832659 + S * 1.610474;
    float g = -L * 6.481468 + M * 17.715578 - S * 2.532642;
    float b = -L * 0.375690 - M * 1.199062 + S * 14.273846;

    return lerp(color, lin2rgb(saturate(float3(r, g, b))), 1.0);
}



float3 main(PS_INPUT Input) : SV_TARGET
{
    float3 color = g_accessibilityTexture.Sample(g_samLinear, Input.vTexcoord);

    int simulationType = 3;
    float3 resultTest;

    switch (simulationType)
    {
    case 1:
        resultTest = protanDeutanFilter(14.443137,114.054902,0.513725,color);
        break;
    case 2:
        resultTest = protanDeutanFilter(37.611765, 90.87451, -2.862745,color);
        break;
    case 3:
        resultTest = tritanFilter(color);
        break;
    default:

       break;
    }

    return resultTest;


    /*Deutan
    int k1 = 37.611765;
    int k2 = 90.87451;
    int k3 = -2.862745;
    */
    /*Protan

    int k1 = 14.443137;
    int k2 = 114.054902;
    int k3 = 0.513725;
    */
    /*Protn Detan

    color = saturate(color);
    float3 c_lin = rgb2lin(color);

    float r_blind = (k1 * c_lin.r + k2 * c_lin.g) / 16448.25098;
    float b_blind = (k3 * c_lin.r - k3 * c_lin.g + 128.498039 * c_lin.b) / 16448.25098;
    r_blind = saturate(r_blind);
    b_blind = saturate(b_blind);

    return lerp(color, lin2rgb(float3(r_blind, r_blind, b_blind)), 1.0);
    */
    /*Tritan
    color = saturate(color);

            float anchor_e0 = 0.05059983 + 0.08585369 + 0.00952420;
            float anchor_e1 = 0.01893033 + 0.08925308 + 0.01370054;
            float anchor_e2 = 0.00292202 + 0.00975732 + 0.07145979;
            float inflection = anchor_e1 / anchor_e0;

            float a1 = -anchor_e2 * 0.007009;
            float b1 = anchor_e2 * 0.0914;
            float c1 = anchor_e0 * 0.007009 - anchor_e1 * 0.0914;
            float a2 = anchor_e1 * 0.3636 - anchor_e2 * 0.2237;
            float b2 = anchor_e2 * 0.1284 - anchor_e0 * 0.3636;
            float c2 = anchor_e0 * 0.2237 - anchor_e1 * 0.1284;

            float3 c_lin = rgb2lin(color);

            float L = (c_lin.r * 0.05059983 + c_lin.g * 0.08585369 + c_lin.b * 0.00952420) / 128.498039;
            float M = (c_lin.r * 0.01893033 + c_lin.g * 0.08925308 + c_lin.b * 0.01370054) / 128.498039;
            float S = (c_lin.r * 0.00292202 + c_lin.g * 0.00975732 + c_lin.b * 0.07145979) / 128.498039;

            float tmp = M / L;

            if (tmp < inflection) S = -(a1 * L + b1 * M) / c1;
            else S = -(a2 * L + b2 * M) / c2;

            float r = L * 30.830854 - M * 29.832659 + S * 1.610474;
            float g = -L * 6.481468 + M * 17.715578 - S * 2.532642;
            float b = -L * 0.375690 - M * 1.199062 + S * 14.273846;

            return lerp(color, lin2rgb(saturate(float3(r, g, b))), 1.0); */


}