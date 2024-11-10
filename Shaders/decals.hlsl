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

struct DecalUniformBufferObject
{
    float4x4 decalProjectionMatrix;
};

cbuffer DecalConstantBuffer : register(b1)
{
   DecalUniformBufferObject decalBO;
};

struct VS_OUT {
    float4 pos : SV_POSITION;
    float3 worldPos : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float4 decalUV : TEXCOORD2;
};

VS_OUT VSMain(float4 inPosition : SV_POSITION)
{
    PSInput result;
    float4x4 MVP = (mul(mul(ubo.projM, ubo.viewM),ubo.modelM)); 
    result.position = mul(MVP, inPosition);
    result.fragPos = mul(ubo.modelM, inPosition);
    return result;
}

//SamplerState mySampler : register(s1);
//Texture2D imageTexture : register(t2);
//Texture2D<float> depthShadowTexture : register(t3);

float4 PSMain(PSInput input) : SV_TARGET
{
    return float4(1,0,0,1);
}