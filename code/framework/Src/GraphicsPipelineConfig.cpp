#include "GraphicsPipelineConfig.h"
#include "Mesh.h"
#include "VulkanInitializers.h"

namespace render {
VkGraphicsPipelineCreateInfo GraphicsPipelineConfigBase::Populate(VkPipelineLayout pipelineLayout,
	VkRenderPass renderPass, uint32_t subPassIndex)
{
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pVertexInputState = &mVertexInputState;	// fill all of the stage
	pipelineInfo.pInputAssemblyState = &mInputAssemblyState;
	pipelineInfo.pViewportState = &mViewportState;
	pipelineInfo.pRasterizationState = &mRasterizationState;
	pipelineInfo.pMultisampleState = &mMultisampleState;
	pipelineInfo.pDepthStencilState = &mDepthStencilState;
	pipelineInfo.pColorBlendState = &mColorBlendState;
	pipelineInfo.pDynamicState = &mDynamicState;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = subPassIndex;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;	// pipeline可以继承，减小创建管线的成本 .flags |= VK_PIPELINE_CREATE_DERIVARIVE_BIT
	pipelineInfo.basePipelineIndex = -1;
	return pipelineInfo;
}

void GraphicsPipelineConfigBase::Fill()
{
	mBindings = {};
	mAttributes = {};
	mVertexInputState = vulkanInitializers::PipelineVertexInputStateCreateInfo(mBindings, mAttributes);

	mInputAssemblyState = vulkanInitializers::PipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	mViewports = { { 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f } };
	mScissors = { { { 0, 0 }, { 1, 1 } } };
	mViewportState = vulkanInitializers::PipelineViewportStateCreateInfo(mViewports, mScissors);

	mRasterizationState = vulkanInitializers::PipelineRasterizationStateCreateInfo();

	mMultisampleState = vulkanInitializers::PipelineMultisampleStateCreateInfo();

	mDepthStencilState = vulkanInitializers::PipelineDepthStencilStateCreateInfo();
		
	mAttachments = { vulkanInitializers::PipelineColorBlendAttachmentState() };
	mColorBlendState = vulkanInitializers::PipelineColorBlendStateCreateInfo(mAttachments);

	mDynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};
	mDynamicState = vulkanInitializers::PipelineDynamicStateCreateInfo(mDynamicStates);

	mIsValid = true;
}

bool GraphicsPipelineConfigBase::AddDynamicState(VkDynamicState state)
{
	if (!mIsValid) {
		return false;
	}
	mDynamicStates.emplace_back(state);
	mDynamicState = vulkanInitializers::PipelineDynamicStateCreateInfo(mDynamicStates);
	return true;
}

bool GraphicsPipelineConfigBase::SetVertexInputBindings(std::vector<VkVertexInputBindingDescription>&& bindings)
{
	if (!mIsValid) {
		return false;
	}
	mBindings = bindings;
	mVertexInputState.vertexBindingDescriptionCount = mBindings.size();
	mVertexInputState.pVertexBindingDescriptions = mBindings.size() == 0 ? nullptr : mBindings.data();
	return true;
}

bool GraphicsPipelineConfigBase::SetVertexInputAttributes(std::vector<VkVertexInputAttributeDescription>&& attributes)
{
	if (!mIsValid) {
		return false;
	}
	mAttributes = attributes;
	mVertexInputState.vertexAttributeDescriptionCount = mAttributes.size();
	mVertexInputState.pVertexAttributeDescriptions = mAttributes.size() == 0 ? nullptr : mAttributes.data();
	return true;
}
}