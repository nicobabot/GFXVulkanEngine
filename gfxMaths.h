#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

struct Vertex
{
	glm::vec3 position;
	glm::vec3 color;

	static VkVertexInputBindingDescription GetBindingDesctiption()
	{
		VkVertexInputBindingDescription vertexInputBindingDescription{};
		vertexInputBindingDescription.binding = 0;
		vertexInputBindingDescription.stride = sizeof(Vertex);
		vertexInputBindingDescription.inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;
		return vertexInputBindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescription()
	{
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		return attributeDescriptions;
	}

};

/*
-Quad-
const std::vector<Vertex> vertices =
{
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};

const std::vector<uint16_t> indices =
{
	{0, 1, 2, 2, 3, 0}
};*/

const std::vector<Vertex> vertices =
{
	{{0.5, -0.5, -0.5}, {1.0f,1.0f,1.0f}},
	{{0.5, -0.5, 0.5}, {1.0f,1.0f,1.0f}},
	{{-0.5, -0.5, 0.5}, {1.0f,1.0f,1.0f}},
	{{-0.5, -0.5, -0.5}, {1.0f,1.0f,1.0f}},
	{{0.5, 0.5, -0.5}, {1.0f,1.0f,1.0f}},
	{{-0.5, 0.5, -0.5}, {1.0f,1.0f,1.0f}},
	{{0.5, 0.5, 0.5}, {1.0f,1.0f,1.0f}},
	{{-0.5, 0.5, 0.5}, {1.0f,1.0f,1.0f}}
};

const std::vector<uint16_t> indices =
{
	{
		//Front
		0,1,2,2,3,0,
		//Back
		//4,6,7,7,5,4,
		5,7,6,6,4,5,
		//Up
		1,6,7,7,2,1,
		//Down
		0,4,5,5,3,0,
		//Right
		4,6,1,1,0,4,
		//Left
		3,2,7,7,5,3
	}
};

struct UniformBufferObject 
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};