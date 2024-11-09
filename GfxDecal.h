#pragma once
#include"gfxMaths.h"
class GfxDecal
{
	glm::mat4 modelMatrix;
	VkImageView imageView;
	VkSampler sampler;
public:

	GfxDecal(const glm::mat4& modelMatrix, const VkImageView& imageView, const VkSampler& sampler)
		: modelMatrix(modelMatrix), imageView(imageView), sampler(sampler)
	{
	}
};

