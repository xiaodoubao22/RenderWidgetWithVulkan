#ifndef __RENDER_PASS_H__
#define __RENDER_PASS_H__

#include <vulkan/vulkan.h>
#include "GraphicsDevice.h"
#include "SwapChain.h"

namespace render {
    class RenderPassTemplate {
    public:
        RenderPassTemplate();
        virtual ~RenderPassTemplate();

        void Init(GraphicsDevice* graphicsDevice, Swapchain* swapchain);
        void CleanUp();

        VkRenderPass Get() { return mRenderPass; }
        std::vector<VkAttachmentDescription>& GetAttachments() { return mAttachments; }

    protected:
        virtual void FillAttachmentDescriptions(std::vector<VkAttachmentDescription>& attachments) = 0;
        virtual void CreateRenderPass(VkRenderPass& renderPass) = 0;

        GraphicsDevice* GetGraphicsDevice() { return mGraphicsDevice; }
        Swapchain* GetSwapchain() { return mSwapchain; }

    private:
        // externel objects
        GraphicsDevice* mGraphicsDevice = nullptr;
        Swapchain* mSwapchain = nullptr;

        // renderpass
        VkRenderPass mRenderPass = VK_NULL_HANDLE;
        std::vector<VkAttachmentDescription> mAttachments;
    };
}


#endif // !__RENDER_PASS_H__

