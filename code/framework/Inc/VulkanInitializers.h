#ifndef __VULKAN_INITIALIZERS_H__
#define __VULKAN_INITIALIZERS_H__

#include <vulkan/vulkan.h>
#include <vector>

namespace vulkanInitializers {
inline VkPipelineVertexInputStateCreateInfo PipelineVertexInputStateCreateInfo(
    std::vector<VkVertexInputBindingDescription>& bindings,
    std::vector<VkVertexInputAttributeDescription>& attributes)
{
    VkPipelineVertexInputStateCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.vertexBindingDescriptionCount = bindings.size();
    info.pVertexBindingDescriptions = bindings.size() == 0 ? nullptr : bindings.data();
    info.vertexAttributeDescriptionCount = attributes.size();
    info.pVertexAttributeDescriptions = attributes.size() == 0 ? nullptr : attributes.data();
    return info;
}

inline VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCreateInfo(
    VkPrimitiveTopology topology, VkBool32 primitiveRestart = VK_FALSE)
{
    VkPipelineInputAssemblyStateCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.topology = topology;
    info.primitiveRestartEnable = primitiveRestart;
    return info;
}

inline VkPipelineTessellationStateCreateInfo PipelineTessellationStateCreateInfo(
    uint32_t patchControlPoints)
{
    VkPipelineTessellationStateCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.patchControlPoints = patchControlPoints;
    return info;
}

inline VkPipelineViewportStateCreateInfo PipelineViewportStateCreateInfo(
    std::vector<VkViewport>& viewPorts, std::vector<VkRect2D>& scissors)
{
    VkPipelineViewportStateCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.viewportCount = viewPorts.size();
    info.pViewports = viewPorts.size() == 0 ? nullptr : viewPorts.data();
    info.scissorCount = scissors.size();
    info.pScissors = scissors.size() == 0 ? nullptr : scissors.data();
    return info;
}

inline VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateCreateInfo()
{
    VkPipelineRasterizationStateCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.depthClampEnable = VK_FALSE;
    info.rasterizerDiscardEnable = VK_FALSE;
    info.polygonMode = VK_POLYGON_MODE_FILL;
    info.cullMode = VK_CULL_MODE_NONE;
    info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    info.depthBiasEnable = VK_FALSE;
    info.depthBiasConstantFactor = 0.0f;
    info.depthBiasClamp = 0.0f;
    info.depthBiasSlopeFactor = 0.0f;
    info.lineWidth = 1.0f;
    return info;
}

inline VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo(
    const VkSampleMask* sampleMask = nullptr)
{
    VkPipelineMultisampleStateCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    info.sampleShadingEnable = VK_FALSE;
    info.minSampleShading = 1.0f;
    info.pSampleMask = sampleMask;
    info.alphaToCoverageEnable = VK_FALSE;
    info.alphaToOneEnable = VK_FALSE;
    return info;
}

inline VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateCreateInfo()
{
    VkPipelineDepthStencilStateCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.depthTestEnable = VK_FALSE;
    info.depthWriteEnable = VK_FALSE;
    info.depthCompareOp = VK_COMPARE_OP_LESS;
    info.depthBoundsTestEnable = VK_FALSE;
    info.minDepthBounds = 0.0f; // Optional
    info.maxDepthBounds = 0.0f; // Optional
    info.stencilTestEnable = VK_FALSE;
    info.front = {}; // Optional
    info.back = {}; // Optional
    return info;
}

inline VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateCreateInfo(
    std::vector<VkPipelineColorBlendAttachmentState>& attachments)
{
    VkPipelineColorBlendStateCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.logicOpEnable = VK_FALSE;
    info.logicOp = VK_LOGIC_OP_CLEAR;
    info.attachmentCount = attachments.size();
    info.pAttachments = attachments.size() == 0 ? nullptr : attachments.data();
    info.blendConstants[0] = 0.0f;
    info.blendConstants[1] = 0.0f;
    info.blendConstants[2] = 0.0f;
    info.blendConstants[3] = 0.0f;
    return info;
}

inline VkPipelineColorBlendAttachmentState PipelineColorBlendAttachmentState()
{
    VkPipelineColorBlendAttachmentState info{};
    info.blendEnable = VK_FALSE;
    info.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    info.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    info.colorBlendOp = VK_BLEND_OP_ADD;
    info.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    info.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    info.alphaBlendOp = VK_BLEND_OP_ADD;
    info.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    return info;
}

inline VkPipelineDynamicStateCreateInfo PipelineDynamicStateCreateInfo(
    std::vector<VkDynamicState>& dynamicStates)
{
    VkPipelineDynamicStateCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.dynamicStateCount = dynamicStates.size();
    info.pDynamicStates = dynamicStates.size() == 0 ? nullptr : dynamicStates.data();
    return info;
}

inline VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding(uint32_t binding,
    VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags stageFlags,
    const VkSampler* pImmutableSamplers = nullptr)
{
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = descriptorType;
    layoutBinding.descriptorCount = descriptorCount;
    layoutBinding.stageFlags = stageFlags;
    layoutBinding.pImmutableSamplers = pImmutableSamplers; // Optional
    return layoutBinding;
}

inline VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo(
    std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
    VkDescriptorSetLayoutCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.bindingCount = bindings.size();
    info.pBindings = bindings.size() == 0 ? nullptr : bindings.data();
    return info;
}

inline VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo(std::vector<VkDescriptorSetLayout>& setLayouts,
    std::vector<VkPushConstantRange>& pushConstantRanges)
{
    VkPipelineLayoutCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.setLayoutCount = setLayouts.size();
    info.pSetLayouts = setLayouts.size() == 0 ? nullptr : setLayouts.data();
    info.pushConstantRangeCount = pushConstantRanges.size();
    info.pPushConstantRanges = pushConstantRanges.size() == 0 ? nullptr : pushConstantRanges.data();
    return info;
}

inline VkRenderPassCreateInfo2 RenderPassCreateInfo2(
    std::vector<VkAttachmentDescription2>& attachments,
    std::vector<VkSubpassDescription2>& subpasses)
{
    VkRenderPassCreateInfo2 info{};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
    info.pNext = nullptr;
    info.flags = 0;
    info.attachmentCount = attachments.size();
    info.pAttachments = attachments.size() == 0 ? nullptr : attachments.data();
    info.subpassCount = subpasses.size();
    info.pSubpasses = subpasses.size() == 0 ? nullptr : subpasses.data();
    info.dependencyCount = 0;
    info.pDependencies = nullptr;
    info.correlatedViewMaskCount = 0;
    info.pCorrelatedViewMasks = nullptr;
    return info;
}

inline void RenderPassCreateInfo2SetArray(VkRenderPassCreateInfo2& info,
    std::vector<VkSubpassDependency2>& dependencies)
{
    info.dependencyCount = dependencies.size();
    info.pDependencies = dependencies.size() == 0 ? nullptr : dependencies.data();
}

inline void RenderPassCreateInfo2SetArray(VkRenderPassCreateInfo2& info,
    std::vector<uint32_t>& correlatedViewMasks)
{
    info.correlatedViewMaskCount = correlatedViewMasks.size();
    info.pCorrelatedViewMasks = correlatedViewMasks.size() == 0 ? nullptr : correlatedViewMasks.data();
}

inline VkAttachmentReference2 AttachmentReference2(uint32_t attachment, VkImageLayout layout,
    VkImageAspectFlags aspectMask = 0)
{
    VkAttachmentReference2 info{};
    info.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
    info.pNext = nullptr;
    info.attachment = attachment;
    info.layout = layout;
    info.aspectMask = aspectMask;
    return info;
}

inline VkFragmentShadingRateAttachmentInfoKHR FragmentShadingRateAttachmentInfoKHR(
    const VkAttachmentReference2* pFragmentShadingRateAttachment,
    VkExtent2D shadingRateAttachmentTexelSize)
{
    VkFragmentShadingRateAttachmentInfoKHR info{};
    info.sType = VK_STRUCTURE_TYPE_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR;
    info.pFragmentShadingRateAttachment = pFragmentShadingRateAttachment;
    info.shadingRateAttachmentTexelSize = shadingRateAttachmentTexelSize;
    return info;
}

inline VkSubpassDescription2 SubpassDescription2(VkPipelineBindPoint pipelineBindPoint,
    VkAttachmentReference2* pColorAttachment,
    const VkAttachmentReference2* pDepthStencilAttachment,
    uint32_t viewMask = 0)
{
    VkSubpassDescription2 info{};
    info.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
    info.pNext = nullptr;
    info.flags = 0;
    info.pipelineBindPoint = pipelineBindPoint;
    info.colorAttachmentCount = 1;
    info.pColorAttachments = pColorAttachment;
    info.pDepthStencilAttachment = pDepthStencilAttachment;
    info.viewMask = viewMask;
    return info;
}

inline VkSubpassDescription2 SubpassDescription2(VkPipelineBindPoint pipelineBindPoint,
    std::vector<VkAttachmentReference2>& colorAttachments,
    const VkAttachmentReference2* pDepthStencilAttachment,
    uint32_t viewMask = 0)
{
    VkSubpassDescription2 info{};
    info.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
    info.pNext = nullptr;
    info.flags = 0;
    info.pipelineBindPoint = pipelineBindPoint;
    info.colorAttachmentCount = colorAttachments.size();
    info.pColorAttachments = colorAttachments.size() == 0 ? nullptr : colorAttachments.data();
    info.pDepthStencilAttachment = pDepthStencilAttachment;
    info.viewMask = viewMask;
    return info;
}

inline VkAttachmentDescription2 AttachmentDescription2(VkFormat format,
    VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT)
{
    VkAttachmentDescription2 info{};
    info.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
    info.pNext = nullptr;
    info.flags = 0;
    info.format = format;
    info.samples = samples;
    info.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    info.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    info.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    info.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    info.finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    return info;
}
inline void AttachmentDescription2SetOp(VkAttachmentDescription2& info,
    VkAttachmentLoadOp load, VkAttachmentStoreOp store)
{
    info.loadOp = load;
    info.storeOp = store;
}
inline void AttachmentDescription2SetStencilOp(VkAttachmentDescription2& info,
    VkAttachmentLoadOp load, VkAttachmentStoreOp store)
{
    info.stencilLoadOp = load;
    info.stencilStoreOp = store;
}
inline void AttachmentDescription2SetLayout(VkAttachmentDescription2& info,
    VkImageLayout initial, VkImageLayout final)
{
    info.initialLayout = initial;
    info.finalLayout = final;
}

inline VkSubpassDependency2 SubpassDependency2(uint32_t srcSubpass, uint32_t dstSubpass)
{
    VkSubpassDependency2 info{};
    info.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
    info.pNext = nullptr;
    info.srcSubpass = srcSubpass;
    info.dstSubpass = dstSubpass;
    return info;
}

inline VkImageCreateInfo ImageCreateInfo(VkImageType imageType,
    VkFormat format, VkExtent3D extent, VkImageUsageFlags usage)
{
    VkImageCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.imageType = imageType;
    info.format = format;
    info.extent = extent;
    info.usage = usage;
    info.flags = 0;
    info.mipLevels = 1;
    info.arrayLayers = 1;
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    return info;
}

inline VkImageViewCreateInfo ImageViewCreateInfo(VkImage image, VkImageViewType viewType,
    VkFormat format, VkImageSubresourceRange subresourceRange)
{
    VkImageViewCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.image = image;
    info.viewType = viewType;
    info.format = format;
    info.subresourceRange = subresourceRange;
    return info;
}

#define VULKAN_INITIALIZERS_FILL_WRITE_DESCRIPTOR_SET \
info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;  \
info.pNext = nullptr;                                 \
info.dstSet = dstSet;                                 \
info.dstBinding = binding;                            \
info.dstArrayElement = dstArrayElement;               \
info.descriptorCount = descriptorCount;               \
info.descriptorType = descriptorType;

inline VkWriteDescriptorSet WriteDescriptorSet(VkDescriptorSet dstSet,
    uint32_t binding, uint32_t dstArrayElement, uint32_t descriptorCount,
    VkDescriptorType descriptorType, const VkDescriptorImageInfo* pImageInfo)
{
    VkWriteDescriptorSet info{};
    VULKAN_INITIALIZERS_FILL_WRITE_DESCRIPTOR_SET
    info.pImageInfo = pImageInfo;
    return info;
}

inline VkWriteDescriptorSet WriteDescriptorSet(VkDescriptorSet dstSet,
    uint32_t binding, uint32_t dstArrayElement, uint32_t descriptorCount,
    VkDescriptorType descriptorType, const VkDescriptorBufferInfo* pBufferInfo)
{
    VkWriteDescriptorSet info{};
    VULKAN_INITIALIZERS_FILL_WRITE_DESCRIPTOR_SET
    info.pBufferInfo = pBufferInfo;
    return info;
}

inline VkWriteDescriptorSet WriteDescriptorSet(VkDescriptorSet dstSet,
    uint32_t binding, uint32_t dstArrayElement, uint32_t descriptorCount,
    VkDescriptorType descriptorType, const VkBufferView* pTexelBufferView)
{
    VkWriteDescriptorSet info{};
    VULKAN_INITIALIZERS_FILL_WRITE_DESCRIPTOR_SET
    info.pTexelBufferView = pTexelBufferView;
    return info;
}

#undef VULKAN_INITIALIZERS_FILL_WRITE_DESCRIPTOR_SET

}       // namespace vulkanInitializers
#endif  // __VULKAN_INITIALIZERS_H__