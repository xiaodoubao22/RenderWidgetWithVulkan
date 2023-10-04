#ifndef __PIPELINE_VARIABLE_SHADING_RATE__
#define __PIPELINE_VARIABLE_SHADING_RATE__

#include "PipelineTemplate.h"

namespace render {
    class PipelineVariableShadingRate : public PipelineTemplate
    {
    public:
        PipelineVariableShadingRate();
        ~PipelineVariableShadingRate();

        virtual std::vector<VkDescriptorPoolSize> PipelineVariableShadingRate::GetDescriptorSize() override;

    private:
        void CreateShaderModules(ShaderModules& shaderModules) override;
        void CreateDescriptorSetLayouts(std::vector<VkDescriptorSetLayout>& descriptorSetLayouts) override;
        void ConfigPipelineInfo(const ShaderModules& shaderModules, PipeLineConfigInfo& pipelineInfo) override;
        void CreatePipeline(const PipeLineConfigInfo& configInfo, const RenderPassInfo& renderPassInfo,
            VkPipelineLayout pipelineLayout, VkPipeline& graphicsPipeline) override;
    };
}


#endif // !__PIPELINE_VARIABLE_SHADING_RATE__


