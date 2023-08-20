#ifndef __PIPELINE_TEST__
#define __PIPELINE_TEST__

#include "PipelineTemplate.h"

namespace render{
    class PipelineTest : public PipelineTemplate
    {
    public:
        PipelineTest();
        ~PipelineTest();

    private:
        virtual void CreateShaderModules(ShaderModules& shaderModules) override;
        virtual void CreatePipeLineLayout(VkPipelineLayout& pipelineLayout) override;
        virtual void ConfigPipeLineInfo(const ShaderModules& shaderModules, PipeLineConfigInfo& pipelineInfo) override;
    };
}


#endif // !__PIPELINE_TEST__
