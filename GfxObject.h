#include <vector>
#include "GfxContext.h"

class Vertex;

class GfxObject
{
	//Transform

	//Rendering
	std::vector<Vertex> vertices;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	std::vector<uint32_t> indices;
	VkBuffer indexBuffer;
	VkDeviceMemory vertexIndexMemory;

	VkDescriptorSetLayout descriptorSetLayout;

	VkPipelineLayout graphicsPipelineLayout;
	VkPipeline graphicsPipeline;

	void CreateVertexBuffer();
	void CreateIndexBuffer();

};

