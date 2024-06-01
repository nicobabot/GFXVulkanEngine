struct PSInput
{
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

PSInput VSMain(float4 inPosition : SV_POSITION)
{
    PSInput result;
    //result.position = mul(ubo.lightSpaceMatrix, mul(ubo.modelM, inPosition));
    result.position = mul(mul(ubo.lightSpaceMatrix, ubo.modelM), inPosition);
    return result;
}

void PSMain(PSInput input) : SV_TARGET
{
   // No output necessary
}