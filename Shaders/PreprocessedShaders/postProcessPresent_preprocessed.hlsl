#line 1 "Shaders/postProcessPresent.hlsl"
struct VSOutput {
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

struct UniformBufferObject
{
    float4x4 modelM;
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
    output.position = float4(inPosition, 1.0f);
    output.texCoord = inTexCoord;
    return output;
}

SamplerState samplerState : register(s1);
Texture2D screenTexture : register(t2);

float4 PSMain(float2 texCoord : TEXCOORD) : SV_TARGET
{
    return screenTexture.Sample(samplerState, texCoord);
}
