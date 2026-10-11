struct VSOutput {
    float4 position : SV_POSITION;  // Output position in screen space (NDC)
    float2 texCoord : TEXCOORD;     // Pass UV coordinates to the fragment shader
};

struct UniformBufferObject
{
    float4x4 viewM;
    float4x4 projM;
    float3 viewPos;
    int debugUtil;
    float deltaTime;
    float4x4 lightSpaceMatrix;
};

cbuffer MyConstantBuffer : register(b0)
{
   UniformBufferObject ubo;
};

VSOutput VSMain(float3 inPosition : SV_POSITION, float3 inColor : COLOR, 
    float2 inTexCoord : TEXCOORD, float3 inNormal : NORMAL)
{
    VSOutput output;
    output.position = float4(inPosition, 1.0f);  // Convert to 4D vector (NDC space)
    output.texCoord = inTexCoord;  // Pass texture coordinates
    return output;
}

SamplerState samplerState : register(s1);  // Sampler for the texture
Texture2D screenTexture : register(t2);  // Input texture (screen texture)
Texture2D<uint> shadingRateTexture : register(t3);

struct PostProcessPushConstants
{
    uint showShadingRateMask;
};

[[vk::push_constant]]
PostProcessPushConstants pc;

float4 PSMain(float2 texCoord : TEXCOORD) : SV_TARGET
{
    float4 sceneColor = screenTexture.Sample(samplerState, texCoord);
    if (pc.showShadingRateMask == 0)
        return sceneColor;

    uint maskWidth, maskHeight;
    shadingRateTexture.GetDimensions(maskWidth, maskHeight);
    uint2 maskCoord = min(uint2(texCoord * float2(maskWidth, maskHeight)), uint2(maskWidth - 1, maskHeight - 1));
    uint rateCode = shadingRateTexture.Load(int3(maskCoord, 0));

    // The attachment stores 0 for 1x1. Non-zero supported codes are coarser rates.
    float3 maskColor = rateCode == 0 ? float3(1.0, 0.08, 0.04) : float3(0.05, 1.0, 0.18);
    return float4(lerp(sceneColor.rgb, maskColor, 0.42), sceneColor.a);
}
