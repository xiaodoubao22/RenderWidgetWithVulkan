#include "GraphicsPipelineConfigInfo.h"
#include "TestMesh.h"
#include "VulkanInitializers.h"
#include "Log.h"

namespace framework {
GraphicsPipelineConfigInfo::GraphicsPipelineConfigInfo()
{
	FillDefault();
};

VkGraphicsPipelineCreateInfo GraphicsPipelineConfigInfo::Populate(VkPipelineLayout pipelineLayout,
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

VkGraphicsPipelineCreateInfo GraphicsPipelineConfigInfo::Populate() const
{
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pVertexInputState = &mVertexInputState;
	pipelineInfo.pInputAssemblyState = &mInputAssemblyState;
	pipelineInfo.pViewportState = &mViewportState;
	pipelineInfo.pRasterizationState = &mRasterizationState;
	pipelineInfo.pMultisampleState = &mMultisampleState;
	pipelineInfo.pDepthStencilState = &mDepthStencilState;
	pipelineInfo.pColorBlendState = &mColorBlendState;
	pipelineInfo.pDynamicState = &mDynamicState;
	pipelineInfo.layout = mPipelineLayout;
	pipelineInfo.renderPass = mRenderPass;
	pipelineInfo.subpass = mSubpassIndex;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;
	return pipelineInfo;
}

bool GraphicsPipelineConfigInfo::AddDynamicState(VkDynamicState state)
{
	mDynamicStates.emplace_back(state);
	mDynamicState = vulkanInitializers::PipelineDynamicStateCreateInfo(mDynamicStates);
	return true;
}

bool GraphicsPipelineConfigInfo::SetVertexInputBindings(std::vector<VkVertexInputBindingDescription>&& bindings)
{
	mBindings = bindings;
	mVertexInputState.vertexBindingDescriptionCount = mBindings.size();
	mVertexInputState.pVertexBindingDescriptions = mBindings.size() == 0 ? nullptr : mBindings.data();
	return true;
}

bool GraphicsPipelineConfigInfo::SetVertexInputAttributes(std::vector<VkVertexInputAttributeDescription>&& attributes)
{
	mAttributes = attributes;
	mVertexInputState.vertexAttributeDescriptionCount = mAttributes.size();
	mVertexInputState.pVertexAttributeDescriptions = mAttributes.size() == 0 ? nullptr : mAttributes.data();
	return true;
}

bool GraphicsPipelineConfigInfo::SetRasterizationSamples(VkSampleCountFlagBits sampleCount)
{
	mMultisampleState.rasterizationSamples = sampleCount;
	return true;
}

bool GraphicsPipelineConfigInfo::SetInputAssemblyState(VkPrimitiveTopology topology, VkBool32 primitiveRestart)
{
	mInputAssemblyState.topology = topology;
	mInputAssemblyState.primitiveRestartEnable = primitiveRestart;
	return true;
}

bool GraphicsPipelineConfigInfo::SetBlendStates(std::vector<VkPipelineColorBlendAttachmentState>& blendAttachments)
{
	mAttachments = blendAttachments;
	mColorBlendState.attachmentCount = mAttachments.size();
	mColorBlendState.pAttachments = mAttachments.empty() ? nullptr : mAttachments.data();
	return true;
}

void GraphicsPipelineConfigInfo::FillDefault()
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
}

}	// namespace framework