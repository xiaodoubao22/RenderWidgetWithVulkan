#ifndef __SHADER_MODULE_FACTORY__
#define __SHADER_MODULE_FACTORY__

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

namespace render {
struct ShaderProgram {
    std::vector<VkPipelineShaderStageCreateInfo> shaderStageInfos = {};
    std::vector<VkShaderModule> shadertModules = {};
    VkDevice device = VK_NULL_HANDLE;
};

struct SpvFilePath {
    std::string str = "";
    VkShaderStageFlagBits stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
};

class ShaderModuleFactory {
public:
    ShaderModuleFactory() {};
    ~ShaderModuleFactory() {};

    static ShaderModuleFactory& GetInstance() {
        static ShaderModuleFactory instance;
        return instance;
    }

    ShaderProgram CreateShaderProgramFromFiles(VkDevice device, std::vector<SpvFilePath> filePaths);

    void DestroyShaderProgram(ShaderProgram& shaderProgram);

private:
    VkShaderModule CreateShaderModule(VkDevice device, const std::vector<char>& code);

};
}

#endif
