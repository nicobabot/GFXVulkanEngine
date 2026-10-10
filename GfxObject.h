#pragma once
#include <vector>
#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>


class Vertex;

// Transform component for GfxObject positioning, rotation, and scaling
struct GfxTransform
{
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Identity quaternion
	glm::vec3 scale = glm::vec3(0.1f, 0.1f, 0.1f);

	// Get the model matrix from transform components
	glm::mat4 GetModelMatrix() const
	{
		glm::mat4 translate = glm::translate(glm::mat4(1.0f), position);
		glm::mat4 rotate = glm::mat4_cast(rotation);
		glm::mat4 scaleM = glm::scale(glm::mat4(1.0f), scale);
		return translate * rotate * scaleM;
	}

	// Set rotation from Euler angles (in radians)
	void SetEulerAngles(glm::vec3 eulerAngles)
	{
		rotation = glm::quat(eulerAngles);
	}

	// Get Euler angles from rotation quaternion (in radians)
	glm::vec3 GetEulerAngles() const
	{
		return glm::eulerAngles(rotation);
	}
};

class GfxObject
{
	public:

	GfxObject(VkPipeline graphicsPipeline, VkPipelineLayout graphicsPipelineLayout, const char* Name = "Unknown");

	void SetDescriptorSetAndLayout(std::vector<VkDescriptorSet> descriptorSet, 
		VkDescriptorSetLayout descriptorSetLayout);
	
	// Transform component
	GfxTransform transform;

	//Rendering
	std::vector<Vertex> vertices;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	std::vector<uint32_t> indices;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	std::vector<VkDescriptorSet> descriptorSet;
	VkDescriptorSetLayout descriptorSetLayout;

	VkPipelineLayout graphicsPipelineLayout;
	VkPipeline graphicsPipeline;

	const char* name;

	void CreateVertexBuffer();
	void CreateIndexBuffer();

};

