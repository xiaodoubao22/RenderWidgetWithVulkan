#include "RenderThread.h"
#include "WindowTemplate.h"
#include "Utils.h"
#include "DebugUtils.h"
#include "VulkanInitializers.h"
#include "ShaderModuleFactory.h"

#include <iostream>
#include <stdexcept>
#include <array>

namespace render {
RenderThread::RenderThread(window::WindowTemplate& w) : RenderBase(w)
{
    mSceneRender = new DrawRotateQuad;
}

RenderThread::~RenderThread()
{
    delete mSceneRender;
}

void RenderThread::OnThreadInit() {
    RenderBase::Init();

    // create render objects
    CreateSyncObjects();
    CreateDepthResources();
    CreatePresentRenderPass();
    CreateFramebuffers();

    RenderInitInfo initInfo{};
    initInfo.presentRenderPass = mPresentRenderPass;
    initInfo.device = mDevice;
    mSceneRender->Init(initInfo);
}

void RenderThread::OnThreadLoop() {
    // 等待前一帧结束(等待队列中的命令执行完)，然后上锁，表示开始画了
    vkWaitForFences(RenderBase::mDevice->Get(), 1, &mInFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(RenderBase::mDevice->Get(), 1, &mInFlightFence);

    // 获取图像
    uint32_t imageIndex;
    if (!mSwapchain->AcquireImage(mImageAvailableSemaphore, imageIndex)) {
        Resize();
    }

    // 记录命令
    RenderInputInfo renderInput{};
    renderInput.presentRenderPass = mPresentRenderPass;
    renderInput.swapchainExtent = mSwapchain->GetExtent();
    renderInput.swapchanFb = mSwapchainFramebuffers[imageIndex];
    std::vector<VkCommandBuffer> commandBuffers = mSceneRender->RecordCommand(renderInput);

    // 提交命令
    std::vector<VkSemaphore> imageAvailiableSemaphore = { mImageAvailableSemaphore };
    std::vector<VkPipelineStageFlags> waitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    std::vector<VkSemaphore> renderFinishedSemaphore = { mRenderFinishedSemaphore };
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = imageAvailiableSemaphore.size();
    submitInfo.pWaitSemaphores = imageAvailiableSemaphore.data();    // 指定要等待的信号量
    submitInfo.pWaitDstStageMask = waitStages.data();      // 指定等待的阶段（颜色附件可写入）zaxG
    submitInfo.commandBufferCount = commandBuffers.size();
    submitInfo.pCommandBuffers = commandBuffers.data();
    submitInfo.signalSemaphoreCount = renderFinishedSemaphore.size();
    submitInfo.pSignalSemaphores = renderFinishedSemaphore.data();    // 指定命令执行完触发mRenderFinishedSemaphore，意思是等我画完再返回交换链
    // 把命令提交到图形队列中，第三个参数指定命令执行完毕后触发mInFlightFence，告诉CPU当前帧画完可以画下一帧了（解锁）
    if (vkQueueSubmit(RenderBase::mDevice->GetGraphicsQueue(), 1, &submitInfo, mInFlightFence) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    // 提交显示
    if (!mSwapchain->QueuePresent(imageIndex, renderFinishedSemaphore)) {
        Resize();
    }

    // 主动重建交换链
    if (Thread::IsFbResized()) {
        Thread::ResetFbResized();
        Resize();
    }
}

void RenderThread::OnThreadDestroy() {
    vkDeviceWaitIdle(mDevice->Get());

    // destroy render objects
    mSceneRender->CleanUp();
    CleanUpFramebuffers();
    CleanUpPresentRenderPass();
    CleanUpDepthResources();
    CleanUpSyncObjects();

    // destroy basic objects
    RenderBase::CleanUp();
}

std::vector<const char*> RenderThread::FillDeviceExtensions()
{
    // 默认只有swapchain扩展
    std::vector<const char*> deviceExt = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    
    // 场景需要的扩展
    mSceneRender->GetRequiredDeviceExtensions(deviceExt);

    return deviceExt;
}

std::vector<const char*> RenderThread::FillInstanceExtensions()
{
    std::vector<const char*> extensions = {
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
    };
    return extensions;
}

void RenderThread::RequestPhysicalDeviceFeatures(PhysicalDevice* physicalDevice) {
    auto& shadingRateCreateInfo = physicalDevice->RequestExtensionsFeatures<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>(
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR);
    shadingRateCreateInfo.pipelineFragmentShadingRate = true;
    shadingRateCreateInfo.attachmentFragmentShadingRate = true;
    shadingRateCreateInfo.primitiveFragmentShadingRate = false;
}

void RenderThread::Resize() {
    vkDeviceWaitIdle(mDevice->Get());

    CleanUpFramebuffers();
    CleanUpDepthResources();

    mSwapchain->Recreate(mWindow.GetWindowExtent());
    CreateDepthResources();
    CreateFramebuffers();
}

void RenderThread::CreateDepthResources() {
    mDepthFormat = FindSupportedFormat(
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    // image
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = mSwapchain->GetExtent().width;
    imageInfo.extent.height = mSwapchain->GetExtent().height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = mDepthFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;		// 可选TILING_LINEAR:行优先 TILLING_OPTIMAL:一种容易访问的结构
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;	// 可选标志，例如3D纹理中避免体素中的空气值
    RenderBase::mDevice->CreateImage(&imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mDepthImage, mDepthImageMemory);

    // imageView
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = mDepthImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = mDepthFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    if (vkCreateImageView(mDevice->Get(), &viewInfo, nullptr, &mDepthImageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }
}

void RenderThread::CleanUpDepthResources() {
    // 销毁深度缓冲
    vkDestroyImageView(mDevice->Get(), mDepthImageView, nullptr);
    vkDestroyImage(mDevice->Get(), mDepthImage, nullptr);
    vkFreeMemory(mDevice->Get(), mDepthImageMemory, nullptr);
}

void RenderThread::CreateFramebuffers() {
    // 对每一个imageView创建帧缓冲
    std::vector<VkImageView> swapChainImageViews = mSwapchain->GetImageViews();
    mSwapchainFramebuffers.resize(swapChainImageViews.size());
    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        std::array<VkImageView, 2> attachments = {
            swapChainImageViews[i],
            mDepthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = mPresentRenderPass;
        framebufferInfo.attachmentCount = attachments.size();	// framebuffer的附件个数
        framebufferInfo.pAttachments = attachments.data();								// framebuffer的附件
        framebufferInfo.width = mSwapchain->GetExtent().width;
        framebufferInfo.height = mSwapchain->GetExtent().height;
        framebufferInfo.layers = 1;		// single pass

        if (vkCreateFramebuffer(mDevice->Get(), &framebufferInfo, nullptr, &mSwapchainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create frambuffer!");
        }
    }
}

void RenderThread::CleanUpFramebuffers() {
    // 销毁frame buffer
    for (auto framebuffer : mSwapchainFramebuffers) {
        vkDestroyFramebuffer(mDevice->Get(), framebuffer, nullptr);
    }
}

void RenderThread::CreateSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;		//（初始化解锁）

    if (vkCreateSemaphore(mDevice->Get(), &semaphoreInfo, nullptr, &mImageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(mDevice->Get(), &semaphoreInfo, nullptr, &mRenderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(mDevice->Get(), &fenceInfo, nullptr, &mInFlightFence) != VK_SUCCESS) {
        throw std::runtime_error("failed to create semaphores!");
    }
}

void RenderThread::CleanUpSyncObjects() {
    vkDestroySemaphore(mDevice->Get(), mImageAvailableSemaphore, nullptr);
    vkDestroySemaphore(mDevice->Get(), mRenderFinishedSemaphore, nullptr);
    vkDestroyFence(mDevice->Get(), mInFlightFence, nullptr);
}

void RenderThread::CreatePresentRenderPass()
{
    // subpass
    VkAttachmentReference2 colorAttachmentRef =
        vulkanInitializers::AttachmentReference2(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkAttachmentReference2 depthAttachmentRef =
        vulkanInitializers::AttachmentReference2(1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    std::vector<VkSubpassDescription2> subpasses = {
        vulkanInitializers::SubpassDescription2(VK_PIPELINE_BIND_POINT_GRAPHICS, &colorAttachmentRef, &depthAttachmentRef),
    };

    std::vector<VkSubpassDependency2> dependencys = { vulkanInitializers::SubpassDependency2(VK_SUBPASS_EXTERNAL, 0) };
    dependencys[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencys[0].srcAccessMask = 0;
    dependencys[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencys[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::vector<VkAttachmentDescription2> mAttachments2(2);
    // 颜色附件
    mAttachments2[0] = vulkanInitializers::AttachmentDescription2(mSwapchain->GetFormat());
    vulkanInitializers::AttachmentDescription2SetOp(mAttachments2[0],
        VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
    vulkanInitializers::AttachmentDescription2SetLayout(mAttachments2[0],
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    // 深度附件
    mAttachments2[1] = vulkanInitializers::AttachmentDescription2(mDepthFormat);
    vulkanInitializers::AttachmentDescription2SetOp(mAttachments2[1],
        VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE);
    vulkanInitializers::AttachmentDescription2SetLayout(mAttachments2[1],
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    VkRenderPassCreateInfo2 renderPassInfo = vulkanInitializers::RenderPassCreateInfo2(mAttachments2, subpasses);
    vulkanInitializers::RenderPassCreateInfo2SetArray(renderPassInfo, dependencys);
    if (vkCreateRenderPass2(mDevice->Get(), &renderPassInfo, nullptr, &mPresentRenderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void RenderThread::CleanUpPresentRenderPass()
{
    vkDestroyRenderPass(mDevice->Get(), mPresentRenderPass, nullptr);
}
}   // namespace render