#include "RenderPassTemplate.h"

namespace render {

    RenderPassTemplate::RenderPassTemplate() {

    }

    RenderPassTemplate::~RenderPassTemplate() {

    }

    void RenderPassTemplate::Init(GraphicsDevice* graphicsDevice, Swapchain* swapchain) {
        mGraphicsDevice = graphicsDevice;
        mSwapchain = swapchain;

        FillAttachmentDescriptions(mAttachments);
        CreateRenderPass(mRenderPass);
    }

    void RenderPassTemplate::CleanUp() {
        vkDestroyRenderPass(mGraphicsDevice->GetDevice(), mRenderPass, nullptr);
    }

}