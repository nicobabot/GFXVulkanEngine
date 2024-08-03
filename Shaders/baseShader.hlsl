struct PSInput
{
    float4 position : SV_POSITION;
	float4 fragColor : COLOR0;
    float3 normal : NORMAL;
    float3 fragTexCoord : TEXCOORD0;
    float4 fragPos : POSITION1;
    float4 viewPos : POSITION2;
    float4 fragPosLightSpace : POSITION3;
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
    float4x4 lightSpaceMatrix;
};

cbuffer MyConstantBuffer : register(b0)
{
   UniformBufferObject ubo;
};

SamplerState mySampler : register(s1);
Texture2D imageTexture : register(t2);
Texture2D<float> depthShadowTexture : register(t3);

#include "brdf.hlsl"
#define SAMPLE_TEXTURE 0
#define SIMPLE_COLOR 0
#define SHADOW_MAP 1
#define SHADOWMAP_DRAW_IN_GEOMETRY 0

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
    result.fragPosLightSpace = mul(ubo.lightSpaceMatrix, result.fragPos);

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

struct DirectionalLight 
{  
    float3 direction;
};  

struct PointLight 
{  
    float3 position; 
    float constantK;
    float linearK;
    float quadraticK; 
};  

float4 FilamentBrdfLight(PSInput input, float3 l)
{

    //Specular BRDF
    //sBRDF = (D * G * F) / (4 * NoV * NoL)
    //D - Normal distribution function (NDF)
    //G - Geomatry shadowing/maksing
    //F - Fresnel, highlight when gazing angles

    float3 n = normalize(input.normal);
    float3 v = normalize(input.viewPos - input.fragPos);
    float3 h = normalize( v + l );
    float roughness = 0.2f;

    float NoL = saturate(dot(n, l));
    float NoV = saturate(dot(n, v));
    float NoH = saturate(dot(n, h));
    float LoH = saturate(dot(l, h));

    float4 diffuseColor = input.fragColor;
#if( SAMPLE_TEXTURE )
    diffuseColor = imageTexture.Sample(mySampler, input.fragTexCoord.rg);
#endif //#if(SAMPLE_TEXTURE)
    float ambientColor = 1.0f;

    float specularStrength = 0.2f;
    float D = D_GGX(NoH, roughness);
    float3 F = F_Schlick_U(LoH, 0.5, 1.0);
    float G = V_GGXCorrelated(NoV, NoL, roughness);
    float3 sBRDF = (D * G)* F  ;
    sBRDF *= specularStrength;

    float3 dBRDF = diffuseColor * Fd_Lambert();

    return float4((dBRDF + sBRDF) * NoL, 1.0f);
}

float GetSpotLightAttenuation(PSInput input, PointLight p)
{
    float d = length(p.position - input.fragPos);
    return (1 / (p.constantK + p.linearK * d + p.quadraticK * d * d));
}

float GetShadowOcclussion(PSInput input, float3 lightDir)
{
    // perform perspective divide
    float3 projCoordsShadows = input.fragPosLightSpace.xyz / input.fragPosLightSpace.w;
    // transform to [0,1] range
    float2 projSample = projCoordsShadows.xy * 0.5f + 0.5f;

    //if(projCoordsShadows.z > 1.0)
      //  return 0.0;

    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords
    float closestDepth = depthShadowTexture.Sample(mySampler, projSample).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoordsShadows.z;
    // check whether current frag pos is in shadow
    //float bias = 0.01f;
    float bias = max(0.05 * (1.0 - dot(input.normal, lightDir)), 0.005);  
    return currentDepth - bias > closestDepth  ? 1.0 : 0.0;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float3 lightDir = float3(-0.32, -0.77, 0.56);
    float4 brdfColor = float4(0,0,0,1);
#if SIMPLE_COLOR
    brdfColor= input.fragColor;
#else // #if SIMPLE_COLOR
    brdfColor = FilamentBrdfLight(input, -lightDir);
#endif // #else // #if SIMPLE_COLOR

    PointLight p;
    p.position = float3(0.0f, 0.0f, 1.5f);
    p.constantK = 1.0f;
    p.linearK = 0.22f;
    p.quadraticK = 0.20f;

    /*float3 pintLightDir = normalize(p.position - input.fragPos);
    float4 pointBrdfColor = FilamentBrdfLight(input, pintLightDir);

    float attenuation = GetSpotLightAttenuation(input, p);
    pointBrdfColor *= attenuation;

    brdfColor += pointBrdfColor;*/


#if SHADOWMAP_DRAW_IN_GEOMETRY
    float3 projCoordsT = input.fragPosLightSpace.xyz / input.fragPosLightSpace.w;
    projCoordsT = projCoordsT * 0.5f + 0.5f;
    float dL = depthShadowTexture.Sample(mySampler, projCoordsT.xy).r;
    return float4(dL.r, dL.r, dL.r, 1.0f);
#endif // #if SHADOWMAP_DRAW_IN_GEOMETRY;

#if SHADOW_MAP
    float shadow = GetShadowOcclussion(input, lightDir);
    //return float4(shadow,shadow,shadow,1.0f);
#else // #if SHADOW_MAP
    float shadow = 0;
#endif // #else //#if SHADOW_MAP
    return float4(brdfColor.rgb * (1-shadow), 1);
}