#ifndef __PIPELINE_DRAW_TEXTURE__
#define __PIPELINE_DRAW_TEXTURE__

#include "PipelineTemplate.h"

namespace render {
    class PipelineDrawTexture : public PipelineTemplate
    {
    public:
        PipelineDrawTexture();
        ~PipelineDrawTexture();

        virtual std::vector<VkDescriptorPoolSize> PipelineDrawTexture::GetDescriptorSize() override;

    private:
        void CreateShaderModules(ShaderModules& shaderModules) override;
        void CreateDescriptorSetLayouts(std::vector<VkDescriptorSetLayout>& descriptorSetLayouts) override;
        void ConfigPipelineInfo(const ShaderModules& shaderModules, PipeLineConfigInfo& pipelineInfo) override;
        void CreatePipeline(const PipeLineConfigInfo& configInfo, const RenderPassInfo& renderPassInfo,
            VkPipelineLayout pipelineLayout, VkPipeline& graphicsPipeline) override;
    };
}


#endif // !__PIPELINE_DRAW_TEXTURE__

