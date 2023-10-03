#include "RenderPassTemplate.h"

namespace render {

    RenderPassTemplate::RenderPassTemplate() {

    }

    RenderPassTemplate::~RenderPassTemplate() {

    }

    void RenderPassTemplate::Init(PhysicalDevice* physicalDevice, Device* device, Swapchain* swapchain) {
        mPhysicalDevice = physicalDevice;
        mDevice = device;
        mSwapchain = swapchain;

        FillAttachmentDescriptions(mAttachments);
        CreateRenderPass(mRenderPass);
    }

    void RenderPassTemplate::CleanUp() {
        vkDestroyRenderPass(mDevice->Get(), mRenderPass, nullptr);

        mPhysicalDevice = nullptr;
        mDevice = nullptr;
        mSwapchain = nullptr;
    }

}