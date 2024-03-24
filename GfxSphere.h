#include "GfxObject.h"
class GfxSphere : GfxObject
{

public:

	void GenerateSphereVertices_Internal(uint32_t numRings, uint32_t numSegments,
		float radius, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
	GfxSphere(VkPipeline graphicsPipeline, VkDescriptorSetLayout descriptorSetLayout);
};

