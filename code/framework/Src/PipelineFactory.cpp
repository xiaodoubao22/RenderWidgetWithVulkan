#include "PipelineFactory.h"
#include "VulkanInitializers.h"
#include "Utils.h"
#include "Log.h"

#undef LOG_TAG
#define LOG_TAG "PipelineFactory"

namespace framework {
PipelineObjecs PipelineFactory::CreateGraphicsPipeline(const GraphicsPipelineConfigInfo& configInfo,
    std::vector<ShaderInfo>& shaderInfos, std::vector<VkDescriptorSetLayoutBinding>& layoutBindings,
    std::vector<VkPushConstantRange>& pushConstantRanges)
{
    std::vector<ShaderFileInfo> shaderFileInfos = {};
    return CreateGraphicsPipelineCoreLogic(configInfo, shaderInfos, shaderFileInfos, false, layoutBindings, pushConstantRanges);
}

PipelineObjecs PipelineFactory::CreateGraphicsPipeline(const GraphicsPipelineConfigInfo& configInfo,
    std::vector<ShaderFileInfo>& shaderFileInfos, std::vector<VkDescriptorSetLayoutBinding>& layoutBindings,
    std::vector<VkPushConstantRange>& pushConstantRanges)
{
    std::vector<ShaderInfo> shaderInfos = {};
    return CreateGraphicsPipelineCoreLogic(configInfo, shaderInfos, shaderFileInfos, true, layoutBindings, pushConstantRanges);
}

PipelineObjecs PipelineFactory::CreateComputePipeline(const ShaderFileInfo& shaderFileInfo, std::vector<VkDescriptorSetLayoutBinding>& layoutBindings,
    std::vector<VkPushConstantRange>& pushConstantRanges)
{
    PipelineObjecs pipeline{};
    if (mDevice == VK_NULL_HANDLE) {
        LOGE("device == null");
        return {};
    }

    // create shader
    if (shaderFileInfo.stage != VK_SHADER_STAGE_COMPUTE_BIT) {
        LOGE("check shader type error");
    }
    VkPipelineShaderStageCreateInfo shaderStageInfo = CreateShaderStage(shaderFileInfo);
    if (shaderStageInfo.module == VK_NULL_HANDLE) {
        LOGE("create shader module failed!");
        return {};
    }

    // discriptor set layout
    pipeline.descriptorSetLayouts = { VK_NULL_HANDLE };
    VkDescriptorSetLayoutCreateInfo layoutInfo = vulkanInitializers::DescriptorSetLayoutCreateInfo(layoutBindings);
    if (vkCreateDescriptorSetLayout(mDevice, &layoutInfo, nullptr, &pipeline.descriptorSetLayouts[0]) != VK_SUCCESS) {
        RetrieveResource({ shaderStageInfo }, pipeline, pipeline.descriptorSetLayouts);
        return {};
    }

    // pipeline layout
    VkPipelineLayoutCreateInfo layoutCreateInfo = vulkanInitializers::PipelineLayoutCreateInfo(
        pipeline.descriptorSetLayouts, pushConstantRanges);
    if (vkCreatePipelineLayout(mDevice, &layoutCreateInfo, nullptr, &pipeline.layout) != VK_SUCCESS) {
        RetrieveResource({ shaderStageInfo }, pipeline, pipeline.descriptorSetLayouts);
        return {};
    }

    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = nullptr;
    pipelineInfo.flags = 0;
    pipelineInfo.layout = pipeline.layout;
    pipelineInfo.stage = shaderStageInfo;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = 0;
    if (vkCreateComputePipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.pipeline) != VK_SUCCESS) {
        LOGE("create shader module failed!");
    }

    vkDestroyShaderModule(mDevice, shaderStageInfo.module, nullptr);

    // save data
    pipeline.descriptorSizes.clear();
    for (int i = 0; i < layoutBindings.size(); i++) {
        VkDescriptorPoolSize size = { layoutBindings[i].descriptorType, layoutBindings[i].descriptorCount };
        pipeline.descriptorSizes.emplace_back(size);
    }

    return pipeline;
}

void PipelineFactory::DestroyPipelineObjecst(PipelineObjecs& pipeline)
{
    vkDestroyPipeline(mDevice, pipeline.pipeline, nullptr);
    vkDestroyPipelineLayout(mDevice, pipeline.layout, nullptr);

    for (int i = 0; i < pipeline.descriptorSetLayouts.size(); i++) {
        vkDestroyDescriptorSetLayout(mDevice, pipeline.descriptorSetLayouts[i], nullptr);
    }
    pipeline = {};
}

PipelineObjecs PipelineFactory::CreateGraphicsPipelineCoreLogic(const GraphicsPipelineConfigInfo& configInfo,
    std::vector<ShaderInfo>& shaderInfos, std::vector<ShaderFileInfo>& shaderFileInfos, bool useShaderFileInfos,
    std::vector<VkDescriptorSetLayoutBinding>& layoutBindings,
    std::vector<VkPushConstantRange>& pushConstantRanges)
{
    PipelineObjecs pipeline{};
    if (mDevice == VK_NULL_HANDLE) {
        return {};
    }

    if (configInfo.mRenderPass == VK_NULL_HANDLE) {
        LOGI("render pass is none on create graphics pipeline");
        return {};
    }

    // create shader
    std::vector<VkPipelineShaderStageCreateInfo> shaderStageInfos =
        useShaderFileInfos ? CreateShaderStages(shaderFileInfos) : CreateShaderStages(shaderInfos);
    if (shaderStageInfos.empty()) {
        return {};
    }

    // discriptor set layout
    pipeline.descriptorSetLayouts = { VK_NULL_HANDLE };
    VkDescriptorSetLayoutCreateInfo layoutInfo = vulkanInitializers::DescriptorSetLayoutCreateInfo(layoutBindings);
    LOGD("Bindings.size=%d", layoutBindings.size());
    if (vkCreateDescriptorSetLayout(mDevice, &layoutInfo, nullptr, &pipeline.descriptorSetLayouts[0]) != VK_SUCCESS) {
        RetrieveResource(shaderStageInfos, pipeline, pipeline.descriptorSetLayouts);
        return {};
    }

    // pipeline layout
    VkPipelineLayoutCreateInfo layoutCreateInfo = vulkanInitializers::PipelineLayoutCreateInfo(
        pipeline.descriptorSetLayouts, pushConstantRanges);
    if (vkCreatePipelineLayout(mDevice, &layoutCreateInfo, nullptr, &pipeline.layout) != VK_SUCCESS) {
        RetrieveResource(shaderStageInfos, pipeline, pipeline.descriptorSetLayouts);
        return {};
    }

    // pipeline
    VkGraphicsPipelineCreateInfo pipelineCreateInfo = configInfo.Populate();
    pipelineCreateInfo.stageCount = shaderStageInfos.size();
    pipelineCreateInfo.pStages = shaderStageInfos.data();
    pipelineCreateInfo.layout = pipeline.layout;

    if (vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline.pipeline) != VK_SUCCESS) {
        RetrieveResource(shaderStageInfos, pipeline, pipeline.descriptorSetLayouts);
        return {};
    }

    // destroy
    DestroyShaderStages(shaderStageInfos);

    // save data
    pipeline.descriptorSizes.clear();
    for (int i = 0; i < layoutBindings.size(); i++) {
        VkDescriptorPoolSize size = { layoutBindings[i].descriptorType, layoutBindings[i].descriptorCount };
        pipeline.descriptorSizes.emplace_back(size);
    }

    return pipeline;
}

std::vector<VkPipelineShaderStageCreateInfo> PipelineFactory::CreateShaderStages(
    const std::vector<ShaderInfo>& shaderInfos)
{
    std::vector<VkPipelineShaderStageCreateInfo> result = {};
    for (auto& shaderInfo : shaderInfos) {
        if (shaderInfo.shaderByteCode.empty()) {
            continue;
        }
        if (mShaderTypeWightList.find(shaderInfo.stage) == mShaderTypeWightList.end()) {
            continue;
        }

        VkShaderModule shaderModule = CreateShaderModule(shaderInfo.shaderByteCode);

        if (shaderModule == VK_NULL_HANDLE) {
            continue;
        }

        VkPipelineShaderStageCreateInfo shaderStage{};
        shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.stage = shaderInfo.stage;
        shaderStage.module = shaderModule;
        shaderStage.pName = "main";
        shaderStage.pSpecializationInfo = nullptr;

        result.emplace_back(shaderStage);
    }
    return result;
}

std::vector<VkPipelineShaderStageCreateInfo> PipelineFactory::CreateShaderStages(const std::vector<ShaderFileInfo>& shaderFileInfos)
{
    std::vector<VkPipelineShaderStageCreateInfo> result = {};
    for (auto& shaderInfo : shaderFileInfos) {
        if (shaderInfo.filePath.empty()) {
            continue;
        }
        if (mShaderTypeWightList.find(shaderInfo.stage) == mShaderTypeWightList.end()) {
            continue;
        }

        auto byteCode = utils::ReadFile(shaderInfo.filePath);
        if (byteCode.empty()) {
            continue;
        }
        VkShaderModule shaderModule = CreateShaderModule(byteCode);

        if (shaderModule == VK_NULL_HANDLE) {
            continue;
        }

        VkPipelineShaderStageCreateInfo shaderStage{};
        shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.stage = shaderInfo.stage;
        shaderStage.module = shaderModule;
        shaderStage.pName = "main";
        shaderStage.pSpecializationInfo = nullptr;

        result.emplace_back(shaderStage);
    }
    return result;
}

VkPipelineShaderStageCreateInfo PipelineFactory::CreateShaderStage(const ShaderFileInfo& shaderFileInfo)
{
    if (shaderFileInfo.filePath.empty()) {
        return {};
    }
    if (mShaderTypeWightList.find(shaderFileInfo.stage) == mShaderTypeWightList.end()) {
        return {};
    }

    auto byteCode = utils::ReadFile(shaderFileInfo.filePath);
    if (byteCode.empty()) {
        return {};
    }
    VkShaderModule shaderModule = CreateShaderModule(byteCode);

    if (shaderModule == VK_NULL_HANDLE) {
        return {};
    }

    VkPipelineShaderStageCreateInfo shaderStage{};
    shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage.stage = shaderFileInfo.stage;
    shaderStage.module = shaderModule;
    shaderStage.pName = "main";
    shaderStage.pSpecializationInfo = nullptr;

    return shaderStage;
}

void PipelineFactory::DestroyShaderStages(std::vector<VkPipelineShaderStageCreateInfo>& shaderStageInfos)
{
    if (mDevice == VK_NULL_HANDLE) {
        return;
    }
    for (auto shaderStageInfo : shaderStageInfos) {
        vkDestroyShaderModule(mDevice, shaderStageInfo.module, nullptr);
    }
    shaderStageInfos = {};
}

VkShaderModule PipelineFactory::CreateShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(mDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }
    return shaderModule;
}

void PipelineFactory::RetrieveResource(std::vector<VkPipelineShaderStageCreateInfo> shaderStageInfos,
    PipelineObjecs& pipeline, std::vector<VkDescriptorSetLayout>& discriptorSetlayouts)
{
    DestroyShaderStages(shaderStageInfos);
    shaderStageInfos = {};

    vkDestroyPipeline(mDevice, pipeline.pipeline, nullptr);
    vkDestroyPipelineLayout(mDevice, pipeline.layout, nullptr);
    pipeline = {};

    for (int i = 0; i < discriptorSetlayouts.size(); i++) {
        vkDestroyDescriptorSetLayout(mDevice, discriptorSetlayouts[i], nullptr);
    }
    discriptorSetlayouts = {};
}
}   // namespace framework