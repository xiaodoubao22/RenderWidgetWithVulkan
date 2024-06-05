#include "RenderThread.h"
#include <iostream>
#include <stdexcept>
#include <array>

#include "WindowTemplate.h"
#include "Utils.h"
#include "DebugUtils.h"
#include "VulkanInitializers.h"
#include "BufferCreator.h"
#include "Log.h"

#undef LOG_TAG
#define LOG_TAG "RenderThread"


namespace framework {
RenderThread::RenderThread(window::WindowTemplate& w) : RenderBase(w)
{
    mSceneRender = CreateSceneRender();
}

RenderThread::~RenderThread()
{
    delete mSceneRender;
}

void RenderThread::OnThreadInit() {
    RenderBase::Init();
    BufferCreator::GetInstance().Init(RenderBase::mDevice);

    mDepthFormat = RenderBase::FindSupportedFormat();

    // create render objects
    CreateSyncObjects();
    CreatePresentRenderPass();
    CreateAttachments();
    CreateFramebuffers();

    RenderInitInfo initInfo{};
    initInfo.presentRenderPass = mPresentRenderPass;
    initInfo.device = mDevice;
    initInfo.swapchainExtent = mSwapchain->GetExtent();
    mSceneRender->Init(initInfo);
}

void RenderThread::OnThreadLoop() {
    // 等待前一帧结束(等待队列中的命令执行完)，然后上锁，表示开始画了
    vkWaitForFences(RenderBase::mDevice->Get(), 1, &mInFlightFence, VK_TRUE, UINT64_MAX);

    // 获取图像
    uint32_t imageIndex;
    if (!mSwapchain->AcquireImage(mImageAvailableSemaphore, imageIndex)) {
        Resize();
        return;
    }

    vkResetFences(RenderBase::mDevice->Get(), 1, &mInFlightFence);

    // 处理输入事件
    InputEventInfo inputEvent{};
    {
        std::unique_lock<std::mutex> lock();
        inputEvent = mInputInfo;
    }
    mSceneRender->ProcessInputEvent(inputEvent);

    // 记录命令
    RenderInputInfo renderInput{};
    renderInput.presentRenderPass = mPresentRenderPass;
    renderInput.swapchainExtent = mSwapchain->GetExtent();
    renderInput.swapchanFb = mSwapchainFramebuffers[imageIndex];
    std::vector<VkCommandBuffer>& commandBuffers = mSceneRender->RecordCommand(renderInput);

    // 提交命令
    std::vector<VkSemaphore> imageAvailiableSemaphore = { mImageAvailableSemaphore };
    std::vector<VkPipelineStageFlags> waitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    std::vector<VkSemaphore> renderFinishedSemaphore = { mRenderFinishedSemaphore };
    if (!commandBuffers.empty()) {
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = imageAvailiableSemaphore.size();
        submitInfo.pWaitSemaphores = imageAvailiableSemaphore.data();    // 指定要等待的信号量
        submitInfo.pWaitDstStageMask = waitStages.data();      // 指定等待的阶段（颜色附件可写入）
        submitInfo.commandBufferCount = commandBuffers.size();
        submitInfo.pCommandBuffers = commandBuffers.data();
        submitInfo.signalSemaphoreCount = renderFinishedSemaphore.size();
        submitInfo.pSignalSemaphores = renderFinishedSemaphore.data();    // 指定命令执行完触发mRenderFinishedSemaphore，意思是等我画完再返回交换链
        // 把命令提交到图形队列中，第三个参数指定命令执行完毕后触发mInFlightFence，告诉CPU当前帧画完可以画下一帧了（解锁）
        if (vkQueueSubmit(RenderBase::mDevice->GetGraphicsQueue(), 1, &submitInfo, mInFlightFence) != VK_SUCCESS) {
            LOGE("failed to submit draw command buffer!");
        }
    }
    else {
        LOGE("commandBuffers empty");
    }

    // 提交显示
    if (!mSwapchain->QueuePresent(imageIndex, renderFinishedSemaphore)) {
        Resize();
    }

    // 主动重建交换链
    if (mFramebufferResized.load()) {
        mFramebufferResized.store(false);
        Resize();
    }
}

void RenderThread::OnThreadDestroy() {
    vkDeviceWaitIdle(mDevice->Get());

    // destroy render objects
    mSceneRender->CleanUp();
    CleanUpFramebuffers();
    CleanUpPresentRenderPass();
    CleanUpAttachments();
    CleanUpSyncObjects();

    BufferCreator::GetInstance().CleanUp();

    // destroy basic objects
    RenderBase::CleanUp();
}

void RenderThread::RequestPhysicalDeviceFeatures(PhysicalDevice* physicalDevice) {
    //auto& shadingRateCreateInfo = physicalDevice->RequestExtensionsFeatures<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>(
    //    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR);
    //shadingRateCreateInfo.pipelineFragmentShadingRate = true;
    //shadingRateCreateInfo.attachmentFragmentShadingRate = true;
    //shadingRateCreateInfo.primitiveFragmentShadingRate = false;
}

void RenderThread::SetMouseButton(int button, int action, int mods)
{
    {
        std::unique_lock<std::mutex> lock(mInputEventMutex);
        if (action == GLFW_PRESS) {
            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                mInputInfo.leftPressFlag = true;
            }
            else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
                mInputInfo.rightPressFlag = true;
            }
            else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
                mInputInfo.middlePressFlag = true;
            }
        }
        else if (action == GLFW_RELEASE) {
            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                mInputInfo.leftPressFlag = false;
            }
            else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
                mInputInfo.rightPressFlag = false;
            }
            else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
                mInputInfo.middlePressFlag = false;
            }
        }
    }
    LOGD("button %d %d %d", button, action, mods);
}

void RenderThread::SetCursorPosChanged(double xpos, double ypos)
{
    {
        std::unique_lock<std::mutex> lock(mInputEventMutex);
        mInputInfo.cursorX = xpos;
        mInputInfo.cursorY = ypos;
    }
    LOGD("pos %f %f", xpos, ypos);
}

void RenderThread::SetKeyEvent(int key, int scancode, int action, int mods)
{
    {
        std::unique_lock<std::mutex> lock(mInputEventMutex);
        mInputInfo.key = key;
        mInputInfo.keyScancode = scancode;
        mInputInfo.keyAction = action;
    }
}

void RenderThread::SetFbResized()
{
    mFramebufferResized.store(true);
}

void RenderThread::Resize() {
    VkExtent2D newExtent = mWindow.GetWindowExtent();
    if (newExtent.width == 0 || newExtent.height == 0) {
        return;
    }

    vkDeviceWaitIdle(mDevice->Get());
    CleanUpFramebuffers();
    CleanUpAttachments();

    mSceneRender->OnResize(newExtent);
    mSwapchain->Recreate(newExtent);
    CreateAttachments();
    CreateFramebuffers();
}

void RenderThread::CreateAttachments() {
    BufferCreator& bufferCreator = BufferCreator::GetInstance();

    // check MSAA config
    VkSampleCountFlagBits maxUsableSampleCount = mPhysicalDevice->GetMaxUsableSampleCount();
    if (GetConfig().presentFb.enableMsaa) {
        if (maxUsableSampleCount <= VK_SAMPLE_COUNT_1_BIT) {
            GetConfig().presentFb.enableMsaa = false;
        }
        GetConfig().presentFb.msaaSampleCount = std::min(maxUsableSampleCount, GetConfig().presentFb.msaaSampleCount);
    }
    else {
        GetConfig().presentFb.msaaSampleCount = VK_SAMPLE_COUNT_1_BIT;
    }

    // depth attahcment
    VkImageCreateInfo imageInfo = vulkanInitializers::ImageCreateInfo(VK_IMAGE_TYPE_2D, mDepthFormat);
    imageInfo.extent = { mSwapchain->GetExtent().width, mSwapchain->GetExtent().height, 1 };
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.samples = GetConfig().presentFb.msaaSampleCount;
    bufferCreator.CreateImage(&imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mDepthImage, mDepthImageMemory);

    VkImageViewCreateInfo viewInfo = vulkanInitializers::ImageViewCreateInfo(mDepthImage, VK_IMAGE_VIEW_TYPE_2D, mDepthFormat);
    viewInfo.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };
    if (vkCreateImageView(mDevice->Get(), &viewInfo, nullptr, &mDepthImageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    // color attachment (if MSAA enable)
    if (GetConfig().presentFb.enableMsaa) {
        VkImageCreateInfo colorImageInfo = vulkanInitializers::ImageCreateInfo(VK_IMAGE_TYPE_2D, RenderBase::mSwapchain->GetFormat());
        colorImageInfo.extent = { mSwapchain->GetExtent().width, mSwapchain->GetExtent().height, 1 };
        colorImageInfo.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        colorImageInfo.samples = GetConfig().presentFb.msaaSampleCount;
        bufferCreator.CreateImage(&colorImageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mColorImage, mColorImageMemory);

        VkImageViewCreateInfo colorViewInfo = vulkanInitializers::ImageViewCreateInfo(
            mColorImage, VK_IMAGE_VIEW_TYPE_2D, RenderBase::mSwapchain->GetFormat());
        colorViewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        if (vkCreateImageView(mDevice->Get(), &colorViewInfo, nullptr, &mColorImageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }
    }
}

void RenderThread::CleanUpAttachments() {
    // destroy color attachemnt
    vkDestroyImageView(mDevice->Get(), mColorImageView, nullptr);
    vkDestroyImage(mDevice->Get(), mColorImage, nullptr);
    vkFreeMemory(mDevice->Get(), mColorImageMemory, nullptr);

    // destroy depth attachemnt
    vkDestroyImageView(mDevice->Get(), mDepthImageView, nullptr);
    vkDestroyImage(mDevice->Get(), mDepthImage, nullptr);
    vkFreeMemory(mDevice->Get(), mDepthImageMemory, nullptr);
}

void RenderThread::CreateFramebuffers() {
    // 对每一个imageView创建帧缓冲
    std::vector<VkImageView> swapChainImageViews = mSwapchain->GetImageViews();
    mSwapchainFramebuffers.resize(swapChainImageViews.size());
    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        std::vector<VkImageView> attachments = {};
        if (GetConfig().presentFb.enableMsaa) {
            attachments = { mColorImageView, mDepthImageView, swapChainImageViews[i]};
        }
        else {
            attachments = { swapChainImageViews[i], mDepthImageView };
        }

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
    if (vkCreateSemaphore(RenderBase::GetDevice(), &semaphoreInfo, nullptr, &mImageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(RenderBase::GetDevice(), &semaphoreInfo, nullptr, &mRenderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(RenderBase::GetDevice(), &fenceInfo, nullptr, &mInFlightFence) != VK_SUCCESS) {
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
    VkAttachmentReference2 colorAttachmentResolveRef = // only use if MSAA enable
        vulkanInitializers::AttachmentReference2(2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    VkSubpassDescription2 subpass0 = vulkanInitializers::SubpassDescription2(VK_PIPELINE_BIND_POINT_GRAPHICS);
    subpass0.colorAttachmentCount = 1;
    subpass0.pColorAttachments = &colorAttachmentRef;
    subpass0.pDepthStencilAttachment = &depthAttachmentRef;
    if (GetConfig().presentFb.enableMsaa) {
        subpass0.pResolveAttachments = &colorAttachmentResolveRef;
    }
    std::vector<VkSubpassDescription2> subpasses = { subpass0 };

    // dependency
    std::vector<VkSubpassDependency2> dependencys = { vulkanInitializers::SubpassDependency2(VK_SUBPASS_EXTERNAL, 0) };
    dependencys[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencys[0].srcAccessMask = 0;
    dependencys[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencys[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    // attachments
    // - color
    VkAttachmentDescription2 colorAttachment = vulkanInitializers::AttachmentDescription2(mSwapchain->GetFormat());
    vulkanInitializers::AttachmentDescription2SetOp(colorAttachment,
        VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
    vulkanInitializers::AttachmentDescription2SetLayout(colorAttachment,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    if (GetConfig().presentFb.enableMsaa) {
        colorAttachment.samples = GetConfig().presentFb.msaaSampleCount;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
    // - depth
    VkAttachmentDescription2 depthAttachment = vulkanInitializers::AttachmentDescription2(mDepthFormat);
    vulkanInitializers::AttachmentDescription2SetOp(depthAttachment,
        VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE);
    vulkanInitializers::AttachmentDescription2SetLayout(depthAttachment,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    if (GetConfig().presentFb.enableMsaa) {
        depthAttachment.samples = GetConfig().presentFb.msaaSampleCount;
    }
    // - resolve attachment (only use if MSAA enable)
    VkAttachmentDescription2 resolveAttachment = vulkanInitializers::AttachmentDescription2(mSwapchain->GetFormat());
    vulkanInitializers::AttachmentDescription2SetOp(resolveAttachment,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE);
    vulkanInitializers::AttachmentDescription2SetLayout(resolveAttachment,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    // create!
    std::vector<VkAttachmentDescription2> mAttachments = { colorAttachment, depthAttachment };
    if (GetConfig().presentFb.enableMsaa) {
        mAttachments.emplace_back(resolveAttachment);
    }
    VkRenderPassCreateInfo2 renderPassInfo = vulkanInitializers::RenderPassCreateInfo2(mAttachments, subpasses);
    vulkanInitializers::RenderPassCreateInfo2SetArray(renderPassInfo, dependencys);
    if (vkCreateRenderPass2(RenderBase::mDevice->Get(), &renderPassInfo, nullptr, &mPresentRenderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void RenderThread::CleanUpPresentRenderPass()
{
    vkDestroyRenderPass(mDevice->Get(), mPresentRenderPass, nullptr);
}
}   // namespace framework