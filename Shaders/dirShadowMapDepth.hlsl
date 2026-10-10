struct PSInput
{
    float4 position : SV_POSITION;
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

struct PushConstants
{
    float4x4 modelM;
};

[[vk::push_constant]]
PushConstants pc;

PSInput VSMain(float4 inPosition : POSITION)
{
    PSInput result;
    float4 worldPos = mul(inPosition, pc.modelM);
    result.position = mul(worldPos, ubo.lightSpaceMatrix);
    return result;

    /*float4 worldPos = mul(inPosition, pc.modelM);
    float4 viewPos  = mul(worldPos, ubo.viewM);
    float4 clipPos  = mul(viewPos, ubo.projM);
    result.position = mul(mul(ubo.lightSpaceMatrix, clipPos), inPosition);*/

    /*float4 worldPos = mul(pc.modelM, inPosition);
    result.position = mul(ubo.lightSpaceMatrix, worldPos);*/

    //float4 worldPos = mul(pc.modelM, inPosition);
    //result.position = mul(ubo.lightSpaceMatrix, worldPos);
    //return result;
}

void PSMain(PSInput input) : SV_TARGET
{
   // No output necessary
}