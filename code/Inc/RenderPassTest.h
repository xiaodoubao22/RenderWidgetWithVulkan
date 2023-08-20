#ifndef __RENDER_PASS_TEST__
#define __RENDER_PASS_TEST__

#include "RenderPassTemplate.h"
namespace render {
    class RenderPassTest : public RenderPassTemplate {
    public:
        RenderPassTest();
        ~RenderPassTest();

        virtual void FillAttachmentDescriptions(std::vector<VkAttachmentDescription>& attachments) override;
        virtual void CreateRenderPass(VkRenderPass& renderPass) override;

        VkFormat GetDepthFormat() { return GetAttachments()[1].format; }

    private:
        VkFormat FindDepthFormat();

    };
}


#endif // !__RENDER_PASS_TEST__

