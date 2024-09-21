struct VSOutput {
    float2 uv : TEXCOORD0;
    float4 position : SV_POSITION;
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

VSOutput VSMain(uint vertexID : SV_VertexID)
{
    VSOutput output;

    // Generate a full-screen triangle with 3 vertices (overdraw minimized)
    float2 positions[3] = {
        float2(-1.0f, -1.0f),
        float2(3.0f, -1.0f),
        float2(-1.0f, 3.0f)
    };

    output.position = float4(positions[vertexID], 0.0f, 1.0f);
    output.uv = (positions[vertexID] + 1.0f) * 0.5f;  // Map [-1, 1] -> [0, 1] for UVs
    return output;
}

SamplerState samplerState : register(s1);  // Sampler for the texture
Texture2D screenTexture : register(t2);  // Input texture (screen texture)

float4 PSMain(float2 uv : TEXCOORD0) : SV_TARGET
{
    return screenTexture.Sample(samplerState, uv);
}