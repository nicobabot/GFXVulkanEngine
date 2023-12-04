#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "ColorsDef.h"
#include <chrono>

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;

	static VkVertexInputBindingDescription GetBindingDesctiption()
	{
		VkVertexInputBindingDescription vertexInputBindingDescription{};
		vertexInputBindingDescription.binding = 0;
		vertexInputBindingDescription.stride = sizeof(Vertex);
		vertexInputBindingDescription.inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;
		return vertexInputBindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescription()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, normal);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, color);

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
	//Front
	{ {0.5, -0.5, -0.5},{0.0f, -1.0f, 0.0f}, RED},
	{{0.5, -0.5, 0.5},	{0.0f, -1.0f, 0.0f}, RED},
	{{-0.5, -0.5, 0.5}, {0.0f, -1.0f, 0.0f}, RED},
	{{-0.5, -0.5, -0.5},{0.0f, -1.0f, 0.0f}, RED},

	//Back
	{{-0.5, 0.5, -0.5}, {0.0f, 1.0f, 0.0f}, GREEN},
	{{-0.5, 0.5, 0.5},	{0.0f, 1.0f, 0.0f}, GREEN},
	{{0.5, 0.5, 0.5},	{0.0f, 1.0f, 0.0f}, GREEN},
	{{0.5, 0.5, -0.5},	{0.0f, 1.0f, 0.0f}, GREEN},

	//Up
	{{0.5, -0.5, 0.5},	{0.0f, 0.0f, 1.0f}, BLUE},
	{{0.5, 0.5, 0.5},	{0.0f, 0.0f, 1.0f}, BLUE},
	{{-0.5, 0.5, 0.5},	{0.0f, 0.0f, 1.0f}, BLUE},
	{{-0.5, -0.5, 0.5},	{0.0f, 0.0f, 1.0f}, BLUE},

	//Down
	{{0.5, 0.5, -0.5},	{0.0f, 0.0f, -1.0f}, YELLOW},
	{{0.5, -0.5, -0.5},	{0.0f, 0.0f, -1.0f}, YELLOW},
	{{-0.5, -0.5, -0.5},{0.0f, 0.0f, -1.0f}, YELLOW},
	{{-0.5, 0.5, -0.5}, {0.0f, 0.0f, -1.0f}, YELLOW},
	
	////Right
	{{0.5, 0.5, -0.5},	{1.0f, 0.0f, 0.0f}, CYAN},
	{{0.5, 0.5, 0.5},	{1.0f, 0.0f, 0.0f}, CYAN},
	{{0.5, -0.5, 0.5},	{1.0f, 0.0f, 0.0f}, CYAN},
	{{0.5, -0.5, -0.5},	{1.0f, 0.0f, 0.0f}, CYAN},
	
	//Left
	{{-0.5, -0.5, -0.5},{-1.0f, 0.0f, 0.0f}, WHITE},
	{{-0.5, -0.5, 0.5}, {-1.0f, 0.0f, 0.0f}, WHITE},
	{{-0.5, 0.5, 0.5},	{-1.0f, 0.0f, 0.0f}, WHITE},
	{{-0.5, 0.5, -0.5}, {-1.0f, 0.0f, 0.0f}, WHITE},

};

const std::vector<uint16_t> indices =
{
	//Front
	0,1,2,2,3,0,
	//Back
	4,5,6,6,7,4,
	//Up
	8,9,10,10,11,8,
	//Down
	12,13,14,14,15,12,
	//Right
	16,17,18,18,19,16,
	//Left
	20,21,22,22,23,20
};

struct UniformBufferObject 
{
	alignas(16) glm::mat4 modelM;
	alignas(16) glm::mat4 viewM;
	alignas(16) glm::mat4 projM;
	glm::vec3 viewPos;
	alignas(4) int debugUtil;
};