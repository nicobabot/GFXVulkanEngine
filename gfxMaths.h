#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "ColorsDef.h"
#include <chrono>
#include <array>
#include <vector>
#include <vulkan/vulkan_core.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#define PI 3.14159265358979323846

struct Vertex
{
	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 texCoord;
	glm::vec3 normal;

	bool operator==(const Vertex otherVertex) const
	{
		return position == otherVertex.position &&
			color == otherVertex.color &&
			texCoord == otherVertex.texCoord &&
			normal == otherVertex.normal;
	}



	static VkVertexInputBindingDescription GetBindingDesctiption()
	{
		VkVertexInputBindingDescription vertexInputBindingDescription{};
		vertexInputBindingDescription.binding = 0;
		vertexInputBindingDescription.stride = sizeof(Vertex);
		vertexInputBindingDescription.inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;
		return vertexInputBindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 4> GetAttributeDescription()
	{
		std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VkFormat::VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex, normal);

		return attributeDescriptions;
	}

};


namespace std
{
	template<> struct hash<Vertex> 
	{
		size_t operator()(Vertex const& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.position) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

const std::vector<Vertex> basicGeometryVertices =
{
	{{0.5,0.5,-0.5},	RED, {1.0f, 1.0f} , {0.0,1.0,0.0}},
	{{-0.5,0.5,-0.5},	RED, {1.0f, 0.0f} , {0.0,1.0,0.0}},
	{{-0.5,0.5,0.5},	RED, {0.0f, 0.0f} , {0.0,1.0,0.0}},
	{{0.5,0.5,0.5},		RED, {0.0f, 1.0f} , {0.0,1.0,0.0}},

	{{-0.5,0.5,-0.5},	CYAN, {1.0f, 1.0f} , {1.0,0.0,0.0}},
	{{-0.5,-0.5,-0.5},	CYAN, {1.0f, 0.0f} , {1.0,0.0,0.0}},
	{{-0.5,-0.5,0.5},	CYAN, {0.0f, 0.0f} , {1.0,0.0,0.0}},
	{{-0.5,0.5,0.5},	CYAN, {0.0f, 1.0f} , {1.0,0.0,0.0}},

	{{-0.5,-0.5,-0.5},	GREEN, {1.0f, 1.0f} , {0.0,-1.0,0.0}},
	{{0.5,-0.5,-0.5},	GREEN, {1.0f, 0.0f} , {0.0,-1.0,0.0}},
	{{0.5,-0.5,0.5},	GREEN, {0.0f, 0.0f} , {0.0,-1.0,0.0}},
	{{-0.5,-0.5,0.5},	GREEN, {0.0f, 1.0f} , {0.0,-1.0,0.0}},

	{{0.5,-0.5,-0.5},	WHITE, {1.0f, 1.0f}	, {-1.0,0.0,0.0}},
	{{0.5,0.5,-0.5},	WHITE, {1.0f, 0.0f}	, {-1.0,0.0,0.0}},
	{{0.5,0.5,0.5},		WHITE, {0.0f, 0.0f}	, {-1.0,0.0,0.0}},
	{{0.5,-0.5,0.5},	WHITE, {0.0f, 1.0f}	, {-1.0,0.0,0.0}},

	{{0.5,-0.5,-0.5},	BLUE, {1.0f, 1.0f} , {0.0,0.0,-1.0}},
	{{-0.5,-0.5,-0.5},	BLUE, {1.0f, 0.0f} , {0.0,0.0,-1.0}},
	{{-0.5,0.5,-0.5},	BLUE, {0.0f, 0.0f} , {0.0,0.0,-1.0}},
	{{0.5,0.5,-0.5},	BLUE, {0.0f, 1.0f} , {0.0,0.0,-1.0}},

	{{0.5,0.5,0.5},		YELLOW, {1.0f, 1.0f} , {0.0,0.0,1.0}},
	{{-0.5,0.5,0.5},	YELLOW, {1.0f, 0.0f} , {0.0,0.0,1.0}},
	{{-0.5,-0.5,0.5},	YELLOW, {0.0f, 0.0f} , {0.0,0.0,1.0}},
	{{0.5,-0.5,0.5},	YELLOW, {0.0f, 1.0f} , {0.0,0.0,1.0}},
};

const std::vector<uint32_t> basicGeometryIndices =
{
	0,1,2,2,3,0,
	4,5,6,6,7,4,
	8,9,10,10,11,8,
	12,13,14,14,15,12,
	16,17,18,18,19,16,
	20,21,22,22,23,20,
};

static std::vector<Vertex> quadVertices =
{
	{{-1.0f, -1.0f, 0.0f}, WHITE, {0.0f, 0.0f}, {0.0,0.0,0.0}},  // Bottom-left
	{{ 1.0f, -1.0f, 0.0f}, WHITE, {1.0f, 0.0f}, {0.0,0.0,0.0}},  // Bottom-right
	{{-1.0f,  1.0f, 0.0f}, WHITE, {0.0f, 1.0f}, {0.0,0.0,0.0}},  // Top-left
	{{ 1.0f,  1.0f, 0.0f}, WHITE, {1.0f, 1.0f}, {0.0,0.0,0.0}}   // Top-right
};

static std::vector<uint32_t> quadIndices = {
	//0, 1, 2, 2, 3, 1
	0, 2, 1, 1, 2, 3
};

/*const std::vector<Vertex> basicGeometryVertices =
{
	//----PLANE----

	{{0.5, -0.5, 0},  RED,	{1.0f, 1.0f} ,{0.0f, 0.0f, -1.0f}},
	{{0.5, 0.5, 0},	 RED,	{1.0f, 0.0f} ,{0.0f, 0.0f, -1.0f}},
	{{-0.5, 0.5, 0},  RED,	{0.0f, 0.0f} ,{0.0f, 0.0f, -1.0f}},
	{{-0.5, -0.5, 0}, RED,	{0.0f, 1.0f} ,{0.0f, 0.0f, -1.0f}},
};

const std::vector<uint32_t> basicGeometryIndices =
{
	//Front
	0,1,2,2,3,0,
};*/

struct UniformBufferObject 
{
	alignas(16) glm::mat4 modelM;
	alignas(16) glm::mat4 viewM;
	alignas(16) glm::mat4 projM;
	glm::vec3 viewPos;
	alignas(4) int debugUtil;
	float deltaTime;
	alignas(16) glm::mat4 lightSpaceMatrix;
};