#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 fragPos;
layout(location = 3) in vec3 viewPosF;
layout(location = 4) in vec2 fragTexCoord;
layout(location = 5) flat in int debugUtilF;

layout(location = 0) out vec4 outColor;
layout(binding = 1) uniform sampler2D texSampler;


float PI = 3.1415926535897932384626433832795;

float D_GGX(float NoH, float roughness)
{
    float a = NoH * roughness;
    float k = roughness / (1 - NoH * NoH + a * a);
    return k * k * (1.0f / PI);
}

float V_GGXCorrelated(float NoV, float NoL, float roughness)
{
    if(NoV == 0 || NoL == 0)
    {
        return 0;
    }

    float a2 = roughness * roughness;
    float GGXV = NoV * sqrt(NoV * NoV * (1 - a2) + a2);
    float GGXL = NoL * sqrt(NoL * NoL * (1 - a2) + a2);
    return 0.5f / (GGXV + GGXL);
}

vec3 F_Schlick(float u, float f0, float f90)
{
    return f0 + (vec3(f90)- f0) * pow(1.0 - u, 5.0);
}

float F_Schlick_U(float u, float f0, float f90) 
{
    return f0 + (f90 - f0) * pow(1.0 - u, 5.0);
}

float Fd_Lambert()
{
    return 1.0 / PI;
}

float Fd_Burley(float NoV, float NoL, float LoH, float roughness) 
{
    float f90 = 0.5f + 2.0f * roughness * LoH * LoH;
    float lightScatter = F_Schlick_U(NoL, 1.0, f90);
    float viewScatter = F_Schlick_U(NoV, 1.0, f90);
    return lightScatter * viewScatter * (1.0 / PI);
}

//Remember
// X : left (1,0,0) | -X : right (-1,0,0)
// Y : forward, closer camera (0,1,0) | -Y : negative forward, far camera (0,-1,0)
// Z : down (0,0,1) | -Z : up (0,0,-1)

void main() 
{
    //Specular BRDF
    //sBRDF = (D * G * F) / (4 * NoV * NoL)
    //D - Normal distribution function (NDF)
    //G - Geomatry shadowing/maksing
    //F - Fresnel, highlight when gazing angles

    vec3 lightPos = vec3(0,0,-5);
    vec3 l = normalize( lightPos - fragPos );
    vec3 v = normalize( viewPosF - fragPos );
    vec3 h = normalize(v + l);
    float roughness = 0.38f;

    float NoL = clamp(dot(normal, l), 0.0f, 1.0f);
    float NoV = clamp(dot(normal, v), 0.0f, 1.0f);
    float NoH = clamp(dot(normal, h), 0.0f, 1.0f);
    float LoH = clamp(dot(l, h), 0.0f, 1.0f);

    vec4 color = texture(texSampler, fragTexCoord);
    float ambientColor = 0.085f;

    float D = D_GGX(NoH, roughness);
    vec3 F = F_Schlick(LoH, 0.0, 1.0);
    float G = V_GGXCorrelated(NoV, NoL, roughness);

    vec3 sBRDF = (D * G) * F ;

    vec3 diffuseColor = vec3(1,1,1);
    //vec3 dBRDF = diffuseColor * Fd_Lambert();
    vec3 dBRDF = color.rgb * 0.05 + Fd_Burley(NoV, NoL, LoH, roughness);

    //outColor = color + vec4(dBRDF,1.0f) + vec4(sBRDF,1.0f);
    outColor = vec4(dBRDF,1.0f);
    //outColor = color * vec4(fragColor,1.0f);;
}