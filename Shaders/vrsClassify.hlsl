Texture2D<float4> sceneColor : register(t1);
RWTexture2D<uint> rateMap : register(u2);

struct VrsPushConstants
{
    uint coarseRateCode;
    uint tileWidth;
    uint tileHeight;
};

[[vk::push_constant]]
VrsPushConstants pc;

[numthreads(8, 8, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID)
{
    uint rateWidth, rateHeight;
    rateMap.GetDimensions(rateWidth, rateHeight);
    if (dispatchId.x >= rateWidth || dispatchId.y >= rateHeight)
        return;

    uint sceneWidth, sceneHeight;
    sceneColor.GetDimensions(sceneWidth, sceneHeight);
    uint2 tileMin = dispatchId.xy * uint2(pc.tileWidth, pc.tileHeight);
    uint2 tileMax = min(tileMin + uint2(pc.tileWidth, pc.tileHeight), uint2(sceneWidth, sceneHeight));

    float3 minColor = float3(1e20, 1e20, 1e20);
    float3 maxColor = float3(-1e20, -1e20, -1e20);
    for (uint y = tileMin.y; y < min(tileMax.y, sceneHeight); ++y)
    {
        for (uint x = tileMin.x; x < min(tileMax.x, sceneWidth); ++x)
        {
            float3 color = sceneColor.Load(int3(x, y, 0)).rgb;
            minColor = min(minColor, color);
            maxColor = max(maxColor, color);
        }
    }

    // Keep full rate at edges/high-frequency detail; request the selected coarse rate on flat tiles.
    float range = max(maxColor.r - minColor.r, max(maxColor.g - minColor.g, maxColor.b - minColor.b));
    rateMap[dispatchId.xy] = range < 0.08 ? pc.coarseRateCode : 0;
}
