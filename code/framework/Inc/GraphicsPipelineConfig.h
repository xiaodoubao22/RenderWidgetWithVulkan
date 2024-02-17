#ifndef __GRAPHICS_PIPELINE_CONFIG_INFO__
#define __GRAPHICS_PIPELINE_CONFIG_INFO__

#include <vector>
#include <string>

#include <vulkan/vulkan.h>

namespace render {
enum GraphicsPipelineType {
    GRAPHICS_PIPELINE_BASE = 0,
    GRAPHICS_PIPELINE_VRS,
};

struct PipelineComponents {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    std::vector<VkDescriptorPoolSize> descriptorSizes = {};
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts = {};
    std::vector<VkPushConstantRange> pushConstantRanges = {};
};

class GraphicsPipelineConfigBase {
public:
    GraphicsPipelineConfigBase() {};
    ~GraphicsPipelineConfigBase() {};

    GraphicsPipelineType GetType()
    {
        return mType;
    }
    bool IsValid()
    {
        return mIsValid;
    }

    virtual VkGraphicsPipelineCreateInfo Populate(VkPipelineLayout pipelineLayout,
        VkRenderPass renderPass, uint32_t subPassIndex = 0);
    virtual VkGraphicsPipelineCreateInfo Populate() const;
    virtual void Fill();

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

public:
    bool mIsValid = false;
    GraphicsPipelineType mType = GRAPHICS_PIPELINE_BASE;

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
}

#endif // !__GRAPHICS_PIPELINE_CONFIG_INFO__

