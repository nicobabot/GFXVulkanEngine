#pragma once
#include"gfxMaths.h"

struct GfxDecalInfo 
{
	glm::vec3 decalPosition;
	glm::vec3 decalDirection;
	glm::vec3 decalUp;
	float decalWidth;
	float decalHeight;
	float decalDepth;
	VkImageView imageView;
};

class GfxDecal
{	
	public:
	GfxDecalInfo info;
	//Projection matrix
	glm::mat4 projectionMat;

public:
	GfxDecal(GfxDecalInfo info)
		: info(info)
	{
		projectionMat = CalculateDecalProjectionMatrix(info.decalPosition, info.decalDirection, info.decalUp, info.decalWidth, info.decalHeight, info.decalDepth);
	}

private:
	glm::mat4 CalculateDecalProjectionMatrix(const glm::vec3& decalPosition, const glm::vec3& decalDirection, const glm::vec3& decalUp, float decalWidth, float decalHeight, float decalDepth);

};

