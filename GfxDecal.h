#pragma once
#include"gfxMaths.h"
class GfxDecal
{
	glm::vec3 decalPosition;
	glm::vec3 decalDirection;
	glm::vec3 decalUp;
	float decalWidth; 
	float decalHeight; 
	float decalDepth;
	//Projection matrix
	glm::mat4 projectionMat;
	// The texture of the decal and sampler
	VkImageView imageView;
	VkSampler sampler;

public:
	GfxDecal(const glm::vec3& decalPosition, const glm::vec3& decalDirection, const glm::vec3& decalUp, float decalWidth, float decalHeight, float decalDepth, const VkImageView& imageView, const VkSampler& sampler)
		: decalPosition(decalPosition), decalDirection(decalDirection), decalUp(decalUp), decalWidth(decalWidth), decalHeight(decalHeight), decalDepth(decalDepth), imageView(imageView), sampler(sampler)
		
	{
		projectionMat = CalculateDecalProjectionMatrix(decalPosition, decalDirection, decalUp, decalWidth, decalHeight, decalDepth);
	}

private:
	glm::mat4 CalculateDecalProjectionMatrix(const glm::vec3& decalPosition, const glm::vec3& decalDirection, const glm::vec3& decalUp, float decalWidth, float decalHeight, float decalDepth);

};

