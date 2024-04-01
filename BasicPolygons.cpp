#include "BasicPolygons.h"
#include "gfxMaths.h"

void GfxSphere::GenerateSphereVertices_Internal(uint32_t numRings, uint32_t numSegments,
	float radius, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
	vertices.clear();
	indices.clear();

	for (uint32_t ring = 0; ring <= numRings; ++ring)
	{
		for (uint32_t segment = 0; segment <= numSegments; ++segment)
		{
			float phi = static_cast<float>(ring) * static_cast<float>(PI) / static_cast<float>(numRings);
			float theta = static_cast<float>(segment) * 2.0f * static_cast<float>(PI) / static_cast<float>(numSegments);

			float x = radius * std::sin(phi) * std::cos(theta);
			float y = radius * std::cos(phi);
			float z = radius * std::sin(phi) * std::sin(theta);

			float nx = std::sin(phi) * std::cos(theta);
			float ny = std::cos(phi);
			float nz = std::sin(phi) * std::sin(theta);

			float u = static_cast<float>(segment) / static_cast<float>(numSegments);
			float v = static_cast<float>(ring) / static_cast<float>(numRings);

			vertices.push_back({ {x, -y, z},WHITE, {u, v}, {nx, -ny, nz} });
		}
	}

	for (uint32_t ring = 0; ring < numRings; ++ring)
	{
		for (uint32_t segment = 0; segment < numSegments; ++segment)
		{
			uint32_t vertexIndex = ring * (numSegments + 1) + segment;

			// Two triangles per quad
			indices.push_back(vertexIndex);
			indices.push_back(vertexIndex + numSegments + 1);
			indices.push_back(vertexIndex + 1);

			indices.push_back(vertexIndex + 1);
			indices.push_back(vertexIndex + numSegments + 1);
			indices.push_back(vertexIndex + numSegments + 2);
		}
	}
}

GfxSphere::GfxSphere(VkPipeline graphicsPipeline, VkPipelineLayout graphicsPipelineLayout) :
	GfxObject(graphicsPipeline, graphicsPipelineLayout)
{
	GenerateSphereVertices_Internal(20, 20, 1.0f, vertices, indices);

	CreateVertexBuffer();
	CreateIndexBuffer();
}


GfxCube::GfxCube(VkPipeline graphicsPipeline, VkPipelineLayout graphicsPipelineLayout)
	: GfxObject(graphicsPipeline, graphicsPipelineLayout)
{
	vertices =
	{
		{{0.5,0.5,-0.5},	RED, {1.0f, 1.0f} , {0.0,1.0,0.0}},
		{{-0.5,0.5,-0.5},	RED, {1.0f, 0.0f} , {0.0,1.0,0.0}},
		{{-0.5,0.5,0.5},	RED, {0.0f, 0.0f} , {0.0,1.0,0.0}},
		{{0.5,0.5,0.5},		RED, {0.0f, 1.0f} , {0.0,1.0,0.0}},

		{{-0.5,0.5,-0.5},	CYAN, {1.0f, 1.0f} , {-1.0,0.0,0.0}},
		{{-0.5,-0.5,-0.5},	CYAN, {1.0f, 0.0f} , {-1.0,0.0,0.0}},
		{{-0.5,-0.5,0.5},	CYAN, {0.0f, 0.0f} , {-1.0,0.0,0.0}},
		{{-0.5,0.5,0.5},	CYAN, {0.0f, 1.0f} , {-1.0,0.0,0.0}},

		{{-0.5,-0.5,-0.5},	GREEN, {1.0f, 1.0f} , {0.0,-1.0,0.0}},
		{{0.5,-0.5,-0.5},	GREEN, {1.0f, 0.0f} , {0.0,-1.0,0.0}},
		{{0.5,-0.5,0.5},	GREEN, {0.0f, 0.0f} , {0.0,-1.0,0.0}},
		{{-0.5,-0.5,0.5},	GREEN, {0.0f, 1.0f} , {0.0,-1.0,0.0}},

		{{0.5,-0.5,-0.5},	WHITE, {1.0f, 1.0f}	, {1.0,0.0,0.0}},
		{{0.5,0.5,-0.5},	WHITE, {1.0f, 0.0f}	, {1.0,0.0,0.0}},
		{{0.5,0.5,0.5},		WHITE, {0.0f, 0.0f}	, {1.0,0.0,0.0}},
		{{0.5,-0.5,0.5},	WHITE, {0.0f, 1.0f}	, {1.0,0.0,0.0}},

		{{0.5,-0.5,-0.5},	BLUE, {1.0f, 1.0f} , {0.0,0.0,-1.0}},
		{{-0.5,-0.5,-0.5},	BLUE, {1.0f, 0.0f} , {0.0,0.0,-1.0}},
		{{-0.5,0.5,-0.5},	BLUE, {0.0f, 0.0f} , {0.0,0.0,-1.0}},
		{{0.5,0.5,-0.5},	BLUE, {0.0f, 1.0f} , {0.0,0.0,-1.0}},

		{{0.5,0.5,0.5},		YELLOW, {1.0f, 1.0f} , {0.0,0.0,1.0}},
		{{-0.5,0.5,0.5},	YELLOW, {1.0f, 0.0f} , {0.0,0.0,1.0}},
		{{-0.5,-0.5,0.5},	YELLOW, {0.0f, 0.0f} , {0.0,0.0,1.0}},
		{{0.5,-0.5,0.5},	YELLOW, {0.0f, 1.0f} , {0.0,0.0,1.0}},
	};

	indices =
	{
		0,1,2,2,3,0,
		4,5,6,6,7,4,
		8,9,10,10,11,8,
		12,13,14,14,15,12,
		16,17,18,18,19,16,
		20,21,22,22,23,20,
	};
}
