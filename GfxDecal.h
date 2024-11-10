#pragma once
#include"gfxMaths.h"
class GfxDecal
{
	// Position, rotation, and scale for decal
	glm::mat4 modelMatrix;
	// The texture of the decal and sampler
	VkImageView imageView;
	VkSampler sampler;

public:

	GfxDecal(const glm::mat4& modelMatrix, const VkImageView& imageView, const VkSampler& sampler)
		: modelMatrix(modelMatrix), imageView(imageView), sampler(sampler)
	{
	}
};

