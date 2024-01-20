#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 fragPos;
layout(location = 3) in vec3 viewPosF;
layout(location = 4) in vec2 fragTexCoord;
layout(location = 5) flat in int debugUtilF;

layout(location = 0) out vec4 outColor;
layout(binding = 1) uniform sampler2D texSampler;

void main() 
{
    //outColor = texture(texSampler, fragTexCoord) * vec4(fragColor,1.0f);
    outColor = vec4(1,0,0,1);
}