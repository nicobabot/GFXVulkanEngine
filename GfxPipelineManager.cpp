#include "GfxPipelineManager.h"
#include "gfxMaths.h"
#include "GfxContext.h"

VkCommandBuffer BeginSingleTimeCommandBuffer_Internal()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    //TODO: Create a separate commandPool (VK_COMMAND_POOL_CREATE_TRANSIENT_BIT) for this short living buffers
    allocInfo.commandPool = gfxCtx->commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(gfxCtx->logicalDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

    return commandBuffer;
}

void EndSingleTimeCommandBuffer_Internal(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.commandBufferCount = 1;
    vkQueueSubmit(gfxCtx->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(gfxCtx->graphicsQueue);

    vkFreeCommandBuffers(gfxCtx->logicalDevice, gfxCtx->commandPool, 1, &commandBuffer);
}

uint32_t FindMemoryType_Internal(uint32_t typeFilter, VkMemoryPropertyFlags memoryFlags)
{
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties{};
    vkGetPhysicalDeviceMemoryProperties(gfxCtx->physicalDevice, &physicalDeviceMemoryProperties);

    for (int i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; ++i)
    {
        if (typeFilter & (1 << i) && memoryFlags &&
            (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & memoryFlags) == memoryFlags)
        {
            return i;
        }
    }

    throw std::runtime_error("Error finding memory type");
}

void CreateGraphicsPipeline_Internal(const GraphicsPipelineInfo& graphicPipelineInfo,
    VkPipelineLayout& graphicPipelineLayout, VkPipeline& graphicPipeline)

{
    //TODO: this should come as argument too
    VkVertexInputBindingDescription vertexBindingDescription = Vertex::GetBindingDesctiption();
    std::array<VkVertexInputAttributeDescription, 4>
        vertexAttributeDescription = Vertex::GetAttributeDescription();

    VkPipelineVertexInputStateCreateInfo vertexStateCreateInfo{};
    vertexStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexStateCreateInfo.vertexBindingDescriptionCount = 1;
    vertexStateCreateInfo.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(vertexAttributeDescription.size());
    vertexStateCreateInfo.pVertexBindingDescriptions = &vertexBindingDescription;
    vertexStateCreateInfo.pVertexAttributeDescriptions = vertexAttributeDescription.data();


    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
    inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(graphicPipelineInfo.viewportExtent.width);
    viewport.height = static_cast<float>(graphicPipelineInfo.viewportExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{};
    scissor.offset = { 0,0 };
    scissor.extent = graphicPipelineInfo.viewportExtent;

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
    viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCreateInfo.scissorCount = 1;
    viewportStateCreateInfo.viewportCount = 1;

    std::vector<VkDynamicState> dynamicStates =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();


    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
    rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationStateCreateInfo.lineWidth = 1.0f;
    rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    //Counter-Clockwise -> Y flip on projection matrix
    rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    //Useful for shadowmaps
    rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
    rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
    rasterizationStateCreateInfo.depthBiasClamp = 0.0f;
    rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
    multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleStateCreateInfo.sampleShadingEnable = VK_TRUE;
    multisampleStateCreateInfo.rasterizationSamples = graphicPipelineInfo.msaaSamples;
    multisampleStateCreateInfo.minSampleShading = .2f; //min fraction for sample shading;closer to one is smoother
    multisampleStateCreateInfo.pSampleMask = nullptr; // Optional
    multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE; // Optional

    VkPipelineDepthStencilStateCreateInfo depthStencilStateAttachment{};
    depthStencilStateAttachment.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilStateAttachment.depthTestEnable = VK_TRUE;
    //depthStencilStateAttachment.stencilTestEnable = VK_TRUE; -> WRONG!
    depthStencilStateAttachment.depthWriteEnable = VK_TRUE;
    depthStencilStateAttachment.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencilStateAttachment.depthBoundsTestEnable = VK_FALSE;
    depthStencilStateAttachment.minDepthBounds = 0.0f;
    depthStencilStateAttachment.maxDepthBounds = 1.0f;

    depthStencilStateAttachment.stencilTestEnable = VK_FALSE;
    depthStencilStateAttachment.front = {};
    depthStencilStateAttachment.back = {};


    VkPipelineColorBlendAttachmentState colorBlendStateAttachment{};
    colorBlendStateAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendStateAttachment.blendEnable = VK_TRUE;
    colorBlendStateAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendStateAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendStateAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    //finalColor.rgb = newAlpha * newColor + (1 - newAlpha) * oldColor;
    colorBlendStateAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendStateAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendStateAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    //finalColor.a = newAlpha.a;

    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
    colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
    colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    colorBlendStateCreateInfo.attachmentCount = 1;
    colorBlendStateCreateInfo.pAttachments = &colorBlendStateAttachment;
    colorBlendStateCreateInfo.blendConstants[0] = 0.0f; // Optional
    colorBlendStateCreateInfo.blendConstants[1] = 0.0f; // Optional
    colorBlendStateCreateInfo.blendConstants[2] = 0.0f; // Optional
    colorBlendStateCreateInfo.blendConstants[3] = 0.0f; // Optional

    //TODO
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &graphicPipelineInfo.descriptorSetLayout;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(gfxCtx->logicalDevice, &pipelineLayoutCreateInfo, nullptr, &graphicPipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating pipeline layuout!");
    }

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
    graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineCreateInfo.stageCount = graphicPipelineInfo.shaderStages.size();
    graphicsPipelineCreateInfo.pStages = graphicPipelineInfo.shaderStages.data();

    graphicsPipelineCreateInfo.pVertexInputState = &vertexStateCreateInfo;
    graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
    graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
    graphicsPipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
    graphicsPipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
    graphicsPipelineCreateInfo.pDepthStencilState = &depthStencilStateAttachment;
    graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
    graphicsPipelineCreateInfo.pDynamicState = &dynamicState;

    graphicsPipelineCreateInfo.layout = graphicPipelineLayout;

    graphicsPipelineCreateInfo.renderPass = graphicPipelineInfo.renderPass;
    graphicsPipelineCreateInfo.subpass = 0;

    graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    graphicsPipelineCreateInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(gfxCtx->logicalDevice, VK_NULL_HANDLE, 1,
        &graphicsPipelineCreateInfo, nullptr, &graphicPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating graphic pipeline!");
    }

}

void CreateBuffer_Internal(VkDeviceSize size, VkBufferUsageFlags usageFlags,
    VkMemoryPropertyFlags memoryFlags, VkBuffer& newBuffer, VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo createBuffer{};
    createBuffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createBuffer.size = size;
    createBuffer.usage = usageFlags;
    createBuffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(gfxCtx->logicalDevice, &createBuffer, nullptr, &newBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating vertex buffer!");
    }

    VkMemoryRequirements bufferMemoryRequirements;
    vkGetBufferMemoryRequirements(gfxCtx->logicalDevice, newBuffer, &bufferMemoryRequirements);

    //FindMemoryType
    VkMemoryAllocateInfo allocateMemory{};
    allocateMemory.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateMemory.allocationSize = bufferMemoryRequirements.size;
    allocateMemory.memoryTypeIndex = FindMemoryType_Internal(bufferMemoryRequirements.memoryTypeBits,
        memoryFlags);

    if (vkAllocateMemory(gfxCtx->logicalDevice, &allocateMemory, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("Error allocating vertex buffer memory");
    }

    vkBindBufferMemory(gfxCtx->logicalDevice, newBuffer, bufferMemory, 0);
}

void CopyBuffer_Internal(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommandBuffer_Internal();

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    EndSingleTimeCommandBuffer_Internal(commandBuffer);
}
