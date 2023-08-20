#include "PipelineTemplate.h"
#include <stdexcept>

namespace render {
    PipelineTemplate::PipelineTemplate() {

    }

    PipelineTemplate::~PipelineTemplate() {

    }

    void PipelineTemplate::Init(GraphicsDevice* graphicsDevice, VkExtent2D windowExtent, const RenderPassInfo& renderPassInfo) {
        mGraphicDevice = graphicsDevice;
        mWindowExtent = windowExtent;

        // shaders
        CreateShaderModules(mShaders);

        // pipeline layout
        CreatePipeLineLayout(mPipelineLayout);

        // pipeline
        PipeLineConfigInfo configInfo{};
        ConfigPipeLineInfo(mShaders, configInfo);
        CreatePipeLine(configInfo, renderPassInfo);

        // destroy shaders
        DestroyShaderModules();
    }

    void PipelineTemplate::CleanUp() {
        vkDestroyPipeline(mGraphicDevice->GetDevice(), mGraphicsPipeline, nullptr);
        vkDestroyPipelineLayout(mGraphicDevice->GetDevice(), mPipelineLayout, nullptr);
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

    void PipelineTemplate::CreatePipeLine(const PipeLineConfigInfo& configInfo, const RenderPassInfo& rednerPassInfo) {
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
        pipelineInfo.layout = mPipelineLayout;
        pipelineInfo.renderPass = rednerPassInfo.renderPass;
        pipelineInfo.subpass = rednerPassInfo.subPassIndex;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;	// pipeline可以继承，减小创建管线的成本 .flags |= VK_PIPELINE_CREATE_DERIVARIVE_BIT
        pipelineInfo.basePipelineIndex = -1;

        if (vkCreateGraphicsPipelines(mGraphicDevice->GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mGraphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }
    }

    void PipelineTemplate::DestroyShaderModules() {
        if (mShaders.vertexShader != VK_NULL_HANDLE) {
            vkDestroyShaderModule(mGraphicDevice->GetDevice(), mShaders.vertexShader, nullptr);
        }
        
        if (mShaders.fragmentShader != VK_NULL_HANDLE) {
            vkDestroyShaderModule(mGraphicDevice->GetDevice(), mShaders.fragmentShader, nullptr);
        }
    }


}