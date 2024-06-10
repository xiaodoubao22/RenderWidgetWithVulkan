#ifndef __PIPELINE_FACTORY__
#define __PIPELINE_FACTORY__

#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <unordered_set>
#include "GraphicsPipelineConfigInfo.h"

namespace framework {
struct ShaderFileInfo {
    std::string filePath = "";
    VkShaderStageFlagBits stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
};

struct ShaderInfo {
    std::vector<char> shaderByteCode = {};
    VkShaderStageFlagBits stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
};

struct PipelineObjecs {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    std::vector<VkDescriptorPoolSize> descriptorSizes = {};
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts = {};
};

struct ShaderInfosSet {

};

class PipelineFactory {
public:
    PipelineFactory() {};
    ~PipelineFactory() {};

    static PipelineFactory& GetInstance() {
        static PipelineFactory instance;
        return instance;
    }

    void SetDevice(VkDevice device) { mDevice = device; }

    PipelineObjecs CreateGraphicsPipeline(const GraphicsPipelineConfigInfo& configInfo,
        std::vector<ShaderInfo>& shaderInfos, std::vector<VkDescriptorSetLayoutBinding>& layoutBindings,
        std::vector<VkPushConstantRange>& pushConstantRanges);

    PipelineObjecs CreateGraphicsPipeline(const GraphicsPipelineConfigInfo& configInfo,
        std::vector<ShaderFileInfo>& shaderFileInfos, std::vector<VkDescriptorSetLayoutBinding>& layoutBindings,
        std::vector<VkPushConstantRange>& pushConstantRanges);

    PipelineObjecs CreateComputePipeline(const ShaderFileInfo& shaderFileInfo, std::vector<VkDescriptorSetLayoutBinding>& layoutBindings,
        std::vector<VkPushConstantRange>& pushConstantRanges);

    void DestroyPipelineObjecst(PipelineObjecs& pipeline);

private:
    PipelineObjecs CreateGraphicsPipelineCoreLogic(const GraphicsPipelineConfigInfo& configInfo,
        std::vector<ShaderInfo>& shaderInfos, std::vector<ShaderFileInfo>& shaderFileInfos, bool useShaderFileInfos,
        std::vector<VkDescriptorSetLayoutBinding>& layoutBindings,
        std::vector<VkPushConstantRange>& pushConstantRanges);
    std::vector<VkPipelineShaderStageCreateInfo> CreateShaderStages(const std::vector<ShaderInfo>& shaderInfos);
    std::vector<VkPipelineShaderStageCreateInfo> CreateShaderStages(const std::vector<ShaderFileInfo>& shaderFileInfos);
    VkPipelineShaderStageCreateInfo CreateShaderStage(const ShaderFileInfo& shaderFileInfo);
    void DestroyShaderStages(std::vector<VkPipelineShaderStageCreateInfo>& shaderStageInfos);
    VkShaderModule CreateShaderModule(const std::vector<char>& code);
    void RetrieveResource(std::vector<VkPipelineShaderStageCreateInfo> shaderStageInfos,
        PipelineObjecs& pipeline, std::vector<VkDescriptorSetLayout>& discriptorSetlayouts);

private:
    VkDevice mDevice = VK_NULL_HANDLE;

    const std::unordered_set<VkShaderStageFlagBits> mShaderTypeWightList = {
        VK_SHADER_STAGE_VERTEX_BIT,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        VK_SHADER_STAGE_COMPUTE_BIT,
        VK_SHADER_STAGE_GEOMETRY_BIT,
    };

};
}   // namespace framework

#endif
