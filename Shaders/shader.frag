#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 fragPos;
layout(location = 3) in vec3 viewPosF;
layout(location = 4) flat in int debugUtilF;

layout(location = 0) out vec4 outColor;

void main() 
{
    vec3 lightPos = vec3(1,0,1);
    vec3 lightDir = normalize(lightPos - fragPos );
    float ambientColor = 0.0085;
    float diffuseColor = max(dot(normal,lightDir), 0);

    float shininess = 32.0f;
    vec3 viewDir = normalize(viewPosF - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float specularColor = pow(max(dot(normal, halfwayDir),0), shininess);

    if(debugUtilF == 1)
    {
        specularColor = 0;
    }

    vec4 finalColor = vec4((ambientColor + diffuseColor) * fragColor + specularColor ,1);
    outColor = finalColor;
}