#ifndef __GRAPHICS_PIPELINE_CONFIG_INFO__
#define __GRAPHICS_PIPELINE_CONFIG_INFO__

#include <vector>
#include <string>

#include <vulkan/vulkan.h>

namespace framework {

class GraphicsPipelineConfigInfo {
public:
    GraphicsPipelineConfigInfo();
    ~GraphicsPipelineConfigInfo() {};

    VkGraphicsPipelineCreateInfo Populate(VkPipelineLayout pipelineLayout,
        VkRenderPass renderPass, uint32_t subPassIndex = 0);
    VkGraphicsPipelineCreateInfo Populate() const;

    // modity config
    bool AddDynamicState(VkDynamicState state);
    bool SetVertexInputBindings(std::vector<VkVertexInputBindingDescription>&& bindings);
    bool SetVertexInputAttributes(std::vector<VkVertexInputAttributeDescription>&& attributes);
    bool SetRasterizationSamples(VkSampleCountFlagBits sampleCount);
    bool SetInputAssemblyState(VkPrimitiveTopology topology, VkBool32 primitiveRestart = VK_FALSE);

    void SetRenderPass(VkRenderPass pass, uint32_t subpassIndex = 0)
    {
        mRenderPass = pass;
        mSubpassIndex = subpassIndex;
    }

    void SetPipelineLayout(VkPipelineLayout pipelineLayout)
    {
        mPipelineLayout = pipelineLayout;
    }

private:
    void FillDefault();

public:
    VkPipelineVertexInputStateCreateInfo mVertexInputState = {};
    std::vector<VkVertexInputBindingDescription> mBindings = {};
    std::vector<VkVertexInputAttributeDescription> mAttributes = {};

    VkPipelineInputAssemblyStateCreateInfo mInputAssemblyState = {};

    VkPipelineViewportStateCreateInfo mViewportState = {};
    std::vector<VkViewport> mViewports = {};
    std::vector<VkRect2D> mScissors = {};

    VkPipelineRasterizationStateCreateInfo mRasterizationState = {};

    VkPipelineMultisampleStateCreateInfo mMultisampleState = {};
    VkSampleMask sampleMask = 0;

    VkPipelineDepthStencilStateCreateInfo mDepthStencilState = {};

    VkPipelineColorBlendStateCreateInfo mColorBlendState = {};
    std::vector<VkPipelineColorBlendAttachmentState> mAttachments = {};

    VkPipelineDynamicStateCreateInfo mDynamicState;
    std::vector<VkDynamicState> mDynamicStates = {};

    VkRenderPass mRenderPass = VK_NULL_HANDLE;
    uint32_t mSubpassIndex = 0;

    VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
};
}   // namespace framework

#endif // !__GRAPHICS_PIPELINE_CONFIG_INFO__

