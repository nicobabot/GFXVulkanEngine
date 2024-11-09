#line 1 "Shaders/decals.hlsl"
struct PSInput
{
    float4 position : SV_POSITION;
    float4 fragPos : POSITION1;
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
    float4x4 MVP = (mul(mul(ubo.projM, ubo.viewM),ubo.modelM));
    result.position = mul(MVP, inPosition);
    result.fragPos = mul(ubo.modelM, inPosition);
    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return float4(1,0,0,1);
}
