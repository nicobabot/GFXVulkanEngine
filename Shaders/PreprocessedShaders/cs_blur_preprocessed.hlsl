#line 1 "Shaders/cs_blur.hlsl"
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

Texture2D<float4> inTexture2D : register(t1);
RWTexture2D<float4> outTexture2D : register(u2);

[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;


    static const int KERNEL_RADIUS = 5;
    static const float kernel[2 * KERNEL_RADIUS + 1] = {
        0.06136, 0.24477, 0.38774, 0.24477, 0.06136,
        0.24477, 0.38774, 0.24477, 0.06136, 0.24477, 0.06136
    };

    uint2 imageSize;
    inTexture2D.GetDimensions(imageSize.x, imageSize.y);

    float4 color = float4(0.0, 0.0, 0.0, 0.0);
    float totalWeight = 0.0;


    for (int i = -KERNEL_RADIUS; i <= KERNEL_RADIUS; ++i)
    {
        int2 coord = int2(DTid.xy) + int2(0, i);

        if (coord.y >= 0 && coord.y < imageSize.y)
        {
            float currentWeight = kernel[i + KERNEL_RADIUS];
            color += inTexture2D.Load(int3(coord, 0)) * currentWeight;
            totalWeight += currentWeight;
        }
    }


    outTexture2D[DTid.xy] = color / totalWeight;
}
