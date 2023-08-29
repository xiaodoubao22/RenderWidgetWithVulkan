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
        virtual void CreateShaderModules(ShaderModules& shaderModules) override;
        virtual void CreateDescriptorSetLayouts(std::vector<VkDescriptorSetLayout>& descriptorSetLayouts) override;
        virtual void ConfigPipelineInfo(const ShaderModules& shaderModules, PipeLineConfigInfo& pipelineInfo) override;
    };
}


#endif // !__PIPELINE_DRAW_TEXTURE__

