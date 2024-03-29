struct PSInput
{
    float4 position : SV_POSITION;
	float4 fragColor : COLOR0;
    float3 normal : NORMAL;
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

Texture2D imageTexture : register(t1);
SamplerState mySampler : register(s1);

#include "brdf.hlsl"

PSInput VSMain(float4 inPosition : SV_POSITION, float3 inColor : COLOR, 
    float2 inTexCoord : TEXCOORD, float3 inNormal : NORMAL)
{
    PSInput result;
    
    float4x4 MVP = (mul(mul(ubo.projM, ubo.viewM),ubo.modelM)); 
    result.position = mul(MVP, inPosition);
    result.fragColor = float4(inColor,1.0f);
    result.normal = normalize(inNormal);
    result.fragPos = mul(ubo.modelM, inPosition);
    //result.viewPosF = ubo.inViewPosF;
    //result.debugUtilF = ubo.debugUtil;
    result.fragTexCoord = float3(inTexCoord, 1.0f);
    result.viewPos = float4(ubo.viewPos,1.0f);

    return result;
}

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

float4 FilamentBrdfLight(PSInput input, float3 l)
{

    //Specular BRDF
    //sBRDF = (D * G * F) / (4 * NoV * NoL)
    //D - Normal distribution function (NDF)
    //G - Geomatry shadowing/maksing
    //F - Fresnel, highlight when gazing angles

    float3 n = input.normal;
    float3 v = normalize(input.viewPos - input.fragPos);
    float3 h = normalize( v + l );
    float roughness = 0.2f;

    float NoL = saturate(dot(n, l));
    float NoV = saturate(dot(n, v));
    float NoH = saturate(dot(n, h));
    float LoH = saturate(dot(l, h));

    float4 diffuseColor = imageTexture.Sample(mySampler, input.fragTexCoord.rg);
    float ambientColor = 1.0f;

    float specularStrength = 0.2f;
    float D = D_GGX(NoH, roughness);
    float3 F = F_Schlick_U(LoH, 0.0, 1.0);
    float G = V_GGXCorrelated(NoV, NoL, roughness);
    float3 sBRDF = (D * G); //* F ;
    sBRDF *= specularStrength;

    float3 dBRDF = diffuseColor * Fd_Lambert();
    //float3 dBRDF = diffuseColor * Fd_Burley(NoV, NoL, LoH, roughness);

    //return float4(dBRDF,1.0f);
    //return float4(dBRDF,1.0f) * NoL;
    return (float4(dBRDF,1.0f) + float4(sBRDF,1.0f)) * NoL;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    //return float4(input.normal,1.0f);

    //float3 lightPos = float3(2,5,-5);
    //float3 lightDir = normalize(lightPos - input.fragPos);
    //float3 lightPos = float3(0,3,0);
    float3 lightDir = float3(-0.32, -0.77, 0.56);
    //float3 lightDir = normalize(input.fragPos.xyz - lightPos);
    //return PhongIlumination(input, float4(lightDir,1.0f));
    return FilamentBrdfLight(input, -lightDir);
    //return DirectionalLight(input.fragColor, input);
}