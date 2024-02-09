#include <iostream>
#include <filesystem>

#include "ShaderModuleFactory.h"
#include "Utils.h"

namespace render {
ShaderProgram ShaderModuleFactory::CreateShaderProgramFromFiles(VkDevice device, std::vector<SpvFilePath> filePaths)
{
    ShaderProgram result{};
    result.shaderStageInfos.clear();
    result.shadertModules.clear();
    for (auto& filePath : filePaths) {
        auto shadeCode = utils::ReadFile(filePath.str);
        if (shadeCode.size() == 0) {
            return {};
        }

        VkShaderModule shaderModule = CreateShaderModule(device, shadeCode);
        if (shaderModule == VK_NULL_HANDLE) {
            return {};
        }

        VkPipelineShaderStageCreateInfo shaderStage{};
        shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.stage = filePath.stage;
        shaderStage.module = shaderModule;
        shaderStage.pName = "main";
        shaderStage.pSpecializationInfo = nullptr;

        result.shaderStageInfos.emplace_back(shaderStage);
        result.shadertModules.emplace_back(shaderModule);
    }

    result.device = device;
    return result;
}

void ShaderModuleFactory::DestroyShaderProgram(ShaderProgram& shaderProgram)
{
    if (shaderProgram.device == VK_NULL_HANDLE) {
        return;
    }
    for (auto shaderModule : shaderProgram.shadertModules) {
        vkDestroyShaderModule(shaderProgram.device, shaderModule, nullptr);
    }
}

VkShaderModule ShaderModuleFactory::CreateShaderModule(VkDevice device, const std::vector<char>& code) {
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
} // namespace render