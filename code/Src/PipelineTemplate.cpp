#include "PipelineTemplate.h"
#include <stdexcept>

namespace render {
    PipelineTemplate::PipelineTemplate() {

    }

    PipelineTemplate::~PipelineTemplate() {

    }

    void PipelineTemplate::Init(Device* Device, VkExtent2D windowExtent, const RenderPassInfo& renderPassInfo) {
        // set externel objects
        mDevice = Device;
        mWindowExtent = windowExtent;

        // shaders
        CreateShaderModules(mShaders);

        // descriptor layouts
        CreateDescriptorSetLayouts(mDescriptorSetLayouts);

        // pipeline layout
        CreatePipelineLayout();

        // pipeline
        PipeLineConfigInfo configInfo{};
        ConfigPipelineInfo(mShaders, configInfo);
        CreatePipeline(configInfo, renderPassInfo, mPipelineLayout, mGraphicsPipeline);

        // destroy shaders
        DestroyShaderModules();
    }

    void PipelineTemplate::CleanUp() {
        vkDestroyPipeline(mDevice->Get(), mGraphicsPipeline, nullptr);

        vkDestroyPipelineLayout(mDevice->Get(), mPipelineLayout, nullptr);
        
        for (auto setLayout : mDescriptorSetLayouts) {
            vkDestroyDescriptorSetLayout(mDevice->Get(), setLayout, nullptr);
        }
    }

    VkShaderModule PipelineTemplate::CreateShaderModule(VkDevice device, const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }
        return shaderModule;
    }

    void PipelineTemplate::CreatePipelineLayout() {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = mDescriptorSetLayouts.size();
        pipelineLayoutInfo.pSetLayouts = mDescriptorSetLayouts.size() == 0 ? nullptr : mDescriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        if (vkCreatePipelineLayout(PipelineTemplate::GetDevice(), &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipline layout!");
        }
    }

    void PipelineTemplate::CreatePipeline(const PipeLineConfigInfo& configInfo, const RenderPassInfo& renderPassInfo,
        VkPipelineLayout pipelineLayout, VkPipeline& graphicsPipeline) {
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = configInfo.shaderStageInfo.size();
        pipelineInfo.pStages = configInfo.shaderStageInfo.data();
        pipelineInfo.pVertexInputState = &configInfo.vertexInputInfo;	// fill all of the stage
        pipelineInfo.pInputAssemblyState = &configInfo.inputAssembly;
        pipelineInfo.pViewportState = &configInfo.viewportState;
        pipelineInfo.pRasterizationState = &configInfo.rasterizer;
        pipelineInfo.pMultisampleState = &configInfo.multisampling;
        pipelineInfo.pDepthStencilState = &configInfo.depthStencil;
        pipelineInfo.pColorBlendState = &configInfo.colorBlending;
        pipelineInfo.pDynamicState = &configInfo.dynamicStateCreateInfo;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = renderPassInfo.renderPass;
        pipelineInfo.subpass = renderPassInfo.subPassIndex;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;	// pipeline可以继承，减小创建管线的成本 .flags |= VK_PIPELINE_CREATE_DERIVARIVE_BIT
        pipelineInfo.basePipelineIndex = -1;
        if (vkCreateGraphicsPipelines(mDevice->Get(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }
    }

    void PipelineTemplate::DestroyShaderModules() {
        if (mShaders.vertexShader != VK_NULL_HANDLE) {
            vkDestroyShaderModule(mDevice->Get(), mShaders.vertexShader, nullptr);
        }
        
        if (mShaders.fragmentShader != VK_NULL_HANDLE) {
            vkDestroyShaderModule(mDevice->Get(), mShaders.fragmentShader, nullptr);
        }
    }


}