#include "GfxObject.h"

class GfxSphere : public GfxObject
{

public:

	void GenerateSphereVertices_Internal(uint32_t numRings, uint32_t numSegments,
		float radius, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
	GfxSphere(VkPipeline graphicsPipeline, VkPipelineLayout graphicsPipelineLayout);
};

class GfxCube : public GfxObject
{
public:
	GfxCube(VkPipeline graphicsPipeline, VkPipelineLayout graphicsPipelineLayout);
};

class GfxPlane : public GfxObject
{
public:
	GfxPlane(VkPipeline graphicsPipeline, VkPipelineLayout graphicsPipelineLayout);
};
