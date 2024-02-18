struct PSInput
{
    float4 position : SV_POSITION;
	float4 fragColor : COLOR0;
    float4 normal : NORMAL;
    float3 fragTexCoord : TEXCOORD0;
    float4 fragPos : POSITION1;
    float4 viewPos : POSITION2;
    //int debugUtilF : POSITION2;
};

struct UniformBufferObject
{
    float4x4 modelM;
    float4x4 viewM;
    float4x4 projM;
    float3 viewPos;
    int debugUtil;
    float deltaTime;
};

cbuffer MyConstantBuffer : register(b0)
{
   UniformBufferObject ubo;
};

//struct MyTexture
//{
    Texture2D imageTexture : register(t1);
    SamplerState mySampler : register(s1);
//};

static const float PI = 3.1415926535897932384626433832795;

float4 DirectionalLight(float4 fragmentColor, PSInput input)
{
    float3 lightPos = float3(0,-5, 0);
    float3 lightDir = normalize(lightPos - input.fragPos);
    float ambientColor = 0.085;
    float NoL = saturate(dot(input.normal.rgb, lightDir));
    float diffuseColor = NoL;

    float3 viewDir = normalize(input.viewPos - input.fragPos);
    float3 halfwayDir = normalize(lightDir + viewDir);
    float shininess = 16.0f;
    float specularStrength = 1.2f;
    float spec = pow(max(dot(input.normal.rgb, halfwayDir), 0), shininess);
    float specularColor = spec * specularStrength;

    float4 finalColor = float4((ambientColor + diffuseColor + specularColor) * fragmentColor.rgb, 1);
    return finalColor;
}

float4 PhongIlumination(PSInput input, float4 lightDir)
{
    float4 texColor = imageTexture.Sample(mySampler, input.fragTexCoord.rg);
    float ambientColor = 0.84f;
    float4 NoL = saturate(dot(input.normal, lightDir));
    float4 diffuseColor = NoL;

    float4 viewDir = normalize(input.viewPos - input.fragPos);
    float4 H = normalize(lightDir + viewDir);
    float shininess = 32.0f;
    float specularStrength = 2.5f;
    float specularColor = pow(max(dot(input.normal, H), 0.0f),shininess) * specularStrength;

    return (diffuseColor + specularColor) * texColor;
}


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

float4 FilamentBrdfLight(PSInput input, float4 l)
{

    //Specular BRDF
    //sBRDF = (D * G * F) / (4 * NoV * NoL)
    //D - Normal distribution function (NDF)
    //G - Geomatry shadowing/maksing
    //F - Fresnel, highlight when gazing angles

    float4 v = normalize(input.viewPos - input.fragPos);
    float4 h = normalize(l + v);
    float roughness = 0.36f;

    float NoL = saturate(dot(input.normal, l));
    float NoV = saturate(dot(input.normal, v));
    float NoH = saturate(dot(input.normal, h));
    float LoH = saturate(dot(l, h));

    float4 diffuseColor = imageTexture.Sample(mySampler, input.fragTexCoord.rg);
    float ambientColor = 0.84f;

    float D = D_GGX(NoH, roughness);
    float3 F = F_Schlick(LoH, 0.0, 1.0);
    float G = V_GGXCorrelated(NoV, NoL, roughness);
    float3 sBRDF = (D * G) * F ;

    //float3 dBRDF = diffuseColor * Fd_Lambert();
    float3 dBRDF = diffuseColor * Fd_Burley(NoV, NoL, LoH, roughness);

    return (float4(dBRDF,1.0f) + float4(sBRDF,1.0f)) * NoL;
}


PSInput VSMain(float4 inPosition : SV_POSITION, float3 inColor : COLOR, 
    float2 inTexCoord : TEXCOORD, float3 inNormal : NORMAL)
{
    PSInput result;
    
    float4x4 MVP = (mul(mul(ubo.projM, ubo.viewM),ubo.modelM)); 
    result.position = mul(MVP, inPosition);
    result.fragColor = float4(inColor,1.0f);
    result.normal = normalize(float4(inNormal, 1.0f));
    result.fragPos = mul(ubo.modelM, inPosition);
    //result.viewPosF = ubo.inViewPosF;
    //result.debugUtilF = ubo.debugUtil;
    result.fragTexCoord = float3(inTexCoord, 1.0f);
    result.viewPos = float4(ubo.viewPos,1.0f);

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float3 lightPos = float3(2,5,-5);
    float3 lightDir = normalize(lightPos - input.fragPos);
    //return PhongIlumination(input, float4(lightDir,1.0f));
    return FilamentBrdfLight(input, float4(lightDir,1.0f));
    //return DirectionalLight(input.fragColor, input);
}