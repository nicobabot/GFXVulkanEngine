#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

void main() 
{
    vec3 lightPos = vec3(0,0,-1);
    vec3 lightDir = normalize(lightPos - fragPos );
    float ambientColor = 0.0085;
    float diffuseColor = max(dot(normal,lightDir), 0);
    vec4 finalColor = vec4((ambientColor + diffuseColor) * fragColor,1);
    outColor = finalColor;
}