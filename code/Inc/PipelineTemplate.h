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

    protected:
        virtual void CreateShaderModules(ShaderModules& shaderModules) = 0;
        virtual void CreatePipeLineLayout(VkPipelineLayout& pipelineLayout) = 0;
        virtual void ConfigPipeLineInfo(const ShaderModules& shaderModules, PipeLineConfigInfo& pipelineInfo) = 0;
        
        VkShaderModule CreateShaderModule(VkDevice device, const std::vector<char>& code);
        VkExtent2D GetWindowExtent() { return mWindowExtent; };
        GraphicsDevice* GetGraphicsDevice() { return mGraphicDevice; };

    private:
        void CreatePipeLine(const PipeLineConfigInfo& configInfo, const RenderPassInfo& rednerPassInfo);
        void DestroyShaderModules();

    private:
        GraphicsDevice* mGraphicDevice = VK_NULL_HANDLE;
        VkExtent2D mWindowExtent = {};

        ShaderModules mShaders = {};
        VkPipeline mGraphicsPipeline = VK_NULL_HANDLE;
        VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
    };
}

#endif // !__PIPELINE_TEMPLATE__

