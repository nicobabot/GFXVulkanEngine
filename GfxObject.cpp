#include "GfxObject.h"

GfxObject::GfxObject(VkPipeline graphicsPipeline, VkPipelineLayout graphicsPipelineLayout, const char* Name)
    : graphicsPipeline(graphicsPipeline), graphicsPipelineLayout(graphicsPipelineLayout), name(Name)
{
}
