#include <vector>
#include <vulkan/vulkan_core.h>


class Vertex;

class GfxObject
{
	public:

	GfxObject(VkPipeline graphicsPipeline, VkDescriptorSetLayout descriptorSetLayout);

	//Transform

	//Rendering
	std::vector<Vertex> vertices;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	std::vector<uint32_t> indices;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	VkDescriptorSetLayout descriptorSetLayout;

	VkPipelineLayout graphicsPipelineLayout;
	VkPipeline graphicsPipeline;

	void CreateVertexBuffer();
	void CreateIndexBuffer();

};

