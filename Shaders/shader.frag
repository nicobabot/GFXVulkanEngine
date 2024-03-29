#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 fragPos;
layout(location = 3) in vec3 viewPosF;
layout(location = 4) in vec2 fragTexCoord;
layout(location = 5) flat in int debugUtilF;

layout(location = 0) out vec4 outColor;
layout(binding = 1) uniform sampler2D texSampler;

//Remember
// X : left (1,0,0) | -X : right (-1,0,0)
// Y : forward, closer camera (0,1,0) | -Y : negative forward, far camera (0,-1,0)
// Z : down (0,0,1) | -Z : up (0,0,-1)

vec4 DirectionalLight(vec4 fragmentColor)
{
    vec3 lightPos = vec3(-5,0,0);
    vec3 lightDir = normalize(lightPos - fragPos );
    float ambientColor = 0.085;
    float diffuseColor = max(dot(normal,lightDir), 0);

    float shininess = 32.0f;
    vec3 viewDir = normalize(viewPosF - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float specularColor = pow(max(dot(normal, halfwayDir),0), shininess);

    vec4 finalColor = vec4((ambientColor + diffuseColor) * fragmentColor.rgb + specularColor ,1);
    return finalColor;
}

vec4 SpotLight(vec4 fragmentColor)
{
    vec3 lightPos = vec3(-3,1,2);
    vec3 lightDir = normalize(lightPos - fragPos );
    float distance = length(lightPos - fragPos );

    float ambientColor = 0.085;
    float diffuseColor = max(dot(normal,lightDir), 0);
    float constant = 1.0f;
    float linear = 0.35f;
    float quadratic = 0.44f;

    float attenuation = 1.0f/(constant + linear * distance + quadratic * (distance*distance));


    ambientColor *= attenuation;
    diffuseColor *= attenuation;

    vec4 finalColor = vec4((ambientColor + diffuseColor) * fragmentColor.rgb ,1);
    return finalColor;

}

void main() 
{
    vec4 finalColor;
    
    if(debugUtilF != 0)
    {
        finalColor = vec4(fragTexCoord, 0.0, 1.0);
    }
    else{
        finalColor = texture(texSampler, fragTexCoord);
    }

    //outColor = SpotLight(finalColor);

    outColor = DirectionalLight(finalColor);
    //outColor = texture(texSampler, fragTexCoord) * vec4(fragColor,1.0f);
}