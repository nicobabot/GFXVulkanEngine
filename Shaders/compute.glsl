#version 450

layout (binding = 0) uniform UniformBufferObject {
    mat4 modelM;
    mat4 viewM;
    mat4 projM;
    vec3 viewPos;
    int debugUtil;
    float deltaTime;
} ubo;

struct TestComputeClass {
    vec2 position;
    vec2 velocity;
    vec4 color;
};

layout(std140, binding = 1) readonly buffer ObjectsSSBOIn {
   TestComputeClass objectsIn[ ];
};

layout(std140, binding = 2) buffer ObjectsSSBOOut {
   TestComputeClass objectsOut[ ];
};

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

void main() 
{
    uint index = gl_GlobalInvocationID.x;  

    TestComputeClass objectIn = objectsIn[index];

    objectsOut[index].position = objectIn.position + objectIn.velocity.xy * ubo.deltaTime;
    objectsOut[index].velocity = objectIn.velocity;
}