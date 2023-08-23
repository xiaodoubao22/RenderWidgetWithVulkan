#ifndef __PIPELINE_TEST__
#define __PIPELINE_TEST__

#include "PipelineTemplate.h"

namespace render{
    class PipelineTest : public PipelineTemplate
    {
    public:
        PipelineTest();
        ~PipelineTest();

        virtual std::vector<VkDescriptorPoolSize> PipelineTest::GetDescriptorSize() override;

    private:
        virtual void CreateShaderModules(ShaderModules& shaderModules) override;
        virtual void CreateDescriptorSetLayouts(std::vector<VkDescriptorSetLayout>& descriptorSetLayouts) override;
        virtual void ConfigPipelineInfo(const ShaderModules& shaderModules, PipeLineConfigInfo& pipelineInfo) override;
    };
}


#endif // !__PIPELINE_TEST__
