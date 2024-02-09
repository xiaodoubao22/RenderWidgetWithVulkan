#ifndef __RENDER_PASS_SHADING_RATE__
#define __RENDER_PASS_SHADING_RATE__

#include "RenderPassTemplate.h"
namespace render {
    class RenderPassShadingRate : public RenderPassTemplate {
    public:
        RenderPassShadingRate();
        ~RenderPassShadingRate();

        virtual void FillAttachmentDescriptions(std::vector<VkAttachmentDescription2>& attachments) override;
        virtual void CreateRenderPass(VkRenderPass& renderPass) override;

        VkFormat GetDepthFormat() { return mDepthFormat; }

        VkRenderPass GetRenderPass2() { return mRenderPass2; }

    private:
        VkFormat mDepthFormat = VK_FORMAT_UNDEFINED;
        VkRenderPass mRenderPass2 = VK_NULL_HANDLE;
    };
}


#endif // !__RENDER_PASS_SHADING_RATE__

