#version 450

layout(binding = 0) uniform UniformBufferObject
{
    mat4 modelM;
    mat4 viewM;
    mat4 projM;
    vec3 viewPos;
    int debugUtil;
    float deltaTime;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec3 fragPos;
layout(location = 3) out vec3 viewPosF;
layout(location = 4) out vec2 outFragTexCoords;
layout(location = 5) out int debugUtilF;

void main() {
    gl_Position = ubo.projM * ubo.viewM * ubo.modelM * vec4(inPosition, 1.0);
    fragColor = inColor;
    normal = normalize(inNormal);
    fragPos = vec3(ubo.modelM * vec4(inPosition, 1.0));
    viewPosF = ubo.viewPos;
    debugUtilF = ubo.debugUtil;
    outFragTexCoords = inTexCoord;
}