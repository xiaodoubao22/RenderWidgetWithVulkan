#ifndef __PIPELINE_TEMPLATE__
#define __PIPELINE_TEMPLATE__

#include <vulkan/vulkan.h>
#include <vector>

#include "GraphicsDevice.h"

namespace render {
    struct ShaderModules {
        VkShaderModule vertexShader = VK_NULL_HANDLE;
        VkShaderModule fragmentShader = VK_NULL_HANDLE;
    };

    struct PipeLineConfigInfo {
        VkViewport viewport = {};
        VkRect2D scissor = {};
        VkPipelineViewportStateCreateInfo viewportState = {};
        std::vector<VkPipelineShaderStageCreateInfo> shaderStageInfo = {};
        std::vector<VkVertexInputBindingDescription> vertexBindingDescriptions = {};
        std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions = {};
        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        VkPipelineMultisampleStateCreateInfo multisampling = {};
        VkPipelineDepthStencilStateCreateInfo depthStencil = {};
        std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments = {};
        VkPipelineColorBlendStateCreateInfo colorBlending = {};
        std::vector<VkDynamicState> dynamicStates = {};
        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
    };

    struct RenderPassInfo {
        VkRenderPass renderPass;
        uint32_t subPassIndex;
    };

    class PipelineTemplate {
    public:
        PipelineTemplate();
        ~PipelineTemplate();

        void Init(GraphicsDevice* graphicsDevice, VkExtent2D windowExtent, const RenderPassInfo& renderPassInfo);
        void CleanUp();

        VkPipeline GetPipeline() { return mGraphicsPipeline; }
        std::vector<VkDescriptorSetLayout> GetDescriptorSetLayouts() { return mDescriptorSetLayouts; }
        VkPipelineLayout GetPipelineLayout() { return mPipelineLayout; }

        virtual std::vector<VkDescriptorPoolSize> GetDescriptorSize() { return {}; }    // 各种类型的descriptor分别需要多少个

    protected:
        virtual void CreateShaderModules(ShaderModules& shaderModules) = 0;
        virtual void CreateDescriptorSetLayouts(std::vector<VkDescriptorSetLayout>& descriptorSetLayouts) = 0;
        virtual void ConfigPipelineInfo(const ShaderModules& shaderModules, PipeLineConfigInfo& pipelineInfo) = 0;
        
        
        VkShaderModule CreateShaderModule(VkDevice device, const std::vector<char>& code);
        VkExtent2D GetWindowExtent() { return mWindowExtent; };
        GraphicsDevice* GetGraphicsDevice() { return mGraphicDevice; };

    private:
        void CreatePipelineLayout();
        void CreatePipeline(const PipeLineConfigInfo& configInfo, const RenderPassInfo& rednerPassInfo);
        void DestroyShaderModules();

    private:
        GraphicsDevice* mGraphicDevice = VK_NULL_HANDLE;
        VkExtent2D mWindowExtent = {};

        ShaderModules mShaders = {};
        VkPipeline mGraphicsPipeline = VK_NULL_HANDLE;

        std::vector<VkDescriptorSetLayout> mDescriptorSetLayouts = {};
        VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
    };
}

#endif // !__PIPELINE_TEMPLATE__

