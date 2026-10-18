#include "GfxObject.h"

GfxObject::GfxObject(VkPipeline graphicsPipeline, VkPipelineLayout graphicsPipelineLayout, const char* Name)
    : graphicsPipeline(graphicsPipeline), graphicsPipelineLayout(graphicsPipelineLayout), name(Name)
{
}

void GfxObject::SetDescriptorSetAndLayout(std::vector<VkDescriptorSet> descriptorSet,
    VkDescriptorSetLayout descriptorSetLayout)
{
    this->descriptorSet = descriptorSet;
    this->descriptorSetLayout = descriptorSetLayout;
}
