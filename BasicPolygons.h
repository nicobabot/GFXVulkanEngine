#include "GfxObject.h"

class GfxSphere : public GfxObject
{

public:

	void GenerateSphereVertices_Internal(uint32_t numRings, uint32_t numSegments,
		float radius, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
	GfxSphere(VkPipeline graphicsPipeline, VkPipelineLayout graphicsPipelineLayout, const char* name);
};

class GfxCube : public GfxObject
{
public:
	GfxCube(VkPipeline graphicsPipeline, VkPipelineLayout graphicsPipelineLayout, const char* name);
};

class GfxPlane : public GfxObject
{
public:
	GfxPlane(VkPipeline graphicsPipeline, VkPipelineLayout graphicsPipelineLayout, const char* name);
};

class GfxModel : public GfxObject
{
public:
	GfxModel(VkPipeline graphicsPipeline, VkPipelineLayout graphicsPipelineLayout, const char* loadModelPath);
};