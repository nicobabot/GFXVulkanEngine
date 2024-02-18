static const float PI = 3.1415926535897932384626433832795;


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

float3 F_Schlick(float u, float f0, float f90)
{
    return f0 + (float3(f90,f90,f90)- f0) * pow(1.0 - u, 5.0);
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