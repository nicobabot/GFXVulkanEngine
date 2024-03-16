#include "ModelLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <unordered_map>

void GenerateSphereVertices(uint32_t numRings, uint32_t numSegments, float radius, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
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

			vertices.push_back({ {x, y, z},	WHITE, {u, v}	, {-nx, ny, -nz} });
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

void ModelLoader::LoadModel()
{
	if (basicGeometry) 
	{
		//Assign vertex/index values to basic geometry
		//vertices = basicGeometryVertices;
		//indices = basicGeometryIndices;

		GenerateSphereVertices(20, 20, 1.0f, vertices, indices);

		return;
	}

	tinyobj::attrib_t attribute;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string errStr, wrnStr;
	if (!tinyobj::LoadObj(&attribute, &shapes, &materials, &errStr, MODEL_PATH.c_str()))
	{
		throw std::runtime_error(errStr);
	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices;
	for(const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices) 
		{
			Vertex vertex{};

			vertex.position = 
			{
				attribute.vertices[3 * index.vertex_index + 0],
				attribute.vertices[3 * index.vertex_index + 1],
				attribute.vertices[3 * index.vertex_index + 2]
			};

			vertex.texCoord =
			{
				attribute.texcoords[2 * index.texcoord_index + 0],
				1 - attribute.texcoords[2 * index.texcoord_index + 1],
			};

			vertex.color = { 1.0f, 1.0f, 1.0f };

			if (uniqueVertices.count(vertex) == 0) 
			{
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}

			indices.push_back(uniqueVertices[vertex]);
		}
	}
}

unsigned char* ModelLoader::LoadTexture(int* width, int* height, int* channels)
{
	return stbi_load(TEXTURE_PATH.c_str(), width, height, channels, STBI_rgb_alpha);
}

void ModelLoader::FreeTextureArrayInfo(unsigned char* pixels)
{
	stbi_image_free(pixels);
}
