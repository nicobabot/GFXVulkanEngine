#pragma once
#include "gfxMaths.h"
#include <vector>
#include <string>

const std::string MODEL_PATH = "Models/viking_room.obj";
const std::string TEXTURE_PATH = "Textures/viking_room.png";

class GfxLoader
{
public:
	void LoadModel();
	unsigned char* LoadTexture(int* width, int* height, int* channels);
	void FreeTextureArrayInfo(unsigned char* pixels);

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
private:
	bool basicGeometry = true;

};

