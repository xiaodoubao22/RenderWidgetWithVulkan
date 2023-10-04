#ifndef __RENDER_PASS_H__
#define __RENDER_PASS_H__

#include <vulkan/vulkan.h>
#include "SwapChain.h"

namespace render {
    class RenderPassTemplate {
    public:
        RenderPassTemplate();
        virtual ~RenderPassTemplate();

        void Init(PhysicalDevice* physicalDevice, Device* device, Swapchain* swapchain);
        void CleanUp();

        VkRenderPass Get() { return mRenderPass; }
        std::vector<VkAttachmentDescription2>& GetAttachments() { return mAttachments; }

    protected:
        virtual void FillAttachmentDescriptions(std::vector<VkAttachmentDescription2>& attachments) = 0;
        virtual void CreateRenderPass(VkRenderPass& renderPass) = 0;

        PhysicalDevice* GetPhisicalDevice() { return mPhysicalDevice; }
        Device* GetDevice() { return mDevice; }
        Swapchain* GetSwapchain() { return mSwapchain; }

    private:
        // externel objects
        PhysicalDevice* mPhysicalDevice = nullptr;
        Device* mDevice = nullptr;
        Swapchain* mSwapchain = nullptr;

        // renderpass
        VkRenderPass mRenderPass = VK_NULL_HANDLE;
        std::vector<VkAttachmentDescription2> mAttachments;
    };
}


#endif // !__RENDER_PASS_H__

