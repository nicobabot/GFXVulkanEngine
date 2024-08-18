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

struct TestComputeClass {
    float2 position;
    float2 velocity;
    float4 color;
};

StructuredBuffer<TestComputeClass> objectsIn : register(t1);
RWStructuredBuffer<TestComputeClass> objectsOut : register(u2);

[numthreads(256, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;//gl_GlobalInvocationID.x;  

    TestComputeClass objectIn = objectsIn[index];

    objectsOut[index].position = objectIn.position + objectIn.velocity.xy * ubo.deltaTime;
    objectsOut[index].velocity = objectIn.velocity;
}