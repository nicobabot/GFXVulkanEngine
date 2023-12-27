#include "ModelLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

//struct Vertex;

void ModelLoader::LoadModel()
{
	if (basicGeometry) 
	{
		//Assign vertex/index values to basic geometry
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

			vertex.texCood =
			{
				attribute.texcoords[2 * index.texcoord_index + 0],
				1 - attribute.texcoords[2 * index.texcoord_index + 1],
			};

			vertex.color = { 1.0f, 1.0f, 1.0f };

			vertices.push_back(vertex);
			indices.push_back(indices.size());
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
