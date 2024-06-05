#ifndef __DRAW_MESH_THREAD_H__
#define __DRAW_MESH_THREAD_H__

#include "Thread.h"
#include "RenderBase.h"
#include "TestMesh.h"
#include "SceneDemoDefs.h"

#include <vector>
#include <vulkan/vulkan.h>

namespace window {
class WindowTemplate;
}

namespace framework {
class RenderThread : public Thread, public RenderBase {
public:
    explicit RenderThread(window::WindowTemplate& w);
    ~RenderThread();

    void SetMouseButton(int button, int action, int mods);
    void SetCursorPosChanged(double xpos, double ypos);
    void SetKeyEvent(int key, int scancode, int action, int mods);

    void SetFbResized();

private:
    // override from Thread
    void OnThreadInit() override;
    void OnThreadLoop() override;
    void OnThreadDestroy() override;

    // override from RenderBase
    void RequestPhysicalDeviceFeatures(PhysicalDevice* physicalDevice) override;

private:
    // ----- rener functions -----
    void Resize();

    // ----- create and clean up ----- 
    void CreateAttachments();
    void CleanUpAttachments();

    void CreateFramebuffers();
    void CleanUpFramebuffers();

    void CreateSyncObjects();
    void CleanUpSyncObjects();

    void CreatePresentRenderPass();
    void CleanUpPresentRenderPass();

private:
    // sync objecs
    VkSemaphore mImageAvailableSemaphore = VK_NULL_HANDLE;
    VkSemaphore mRenderFinishedSemaphore = VK_NULL_HANDLE;
    VkFence mInFlightFence = VK_NULL_HANDLE;

    // present fb depth attahcment
    VkFormat mDepthFormat = VK_FORMAT_UNDEFINED;
    VkImage mDepthImage = VK_NULL_HANDLE;
    VkDeviceMemory mDepthImageMemory = VK_NULL_HANDLE;
    VkImageView mDepthImageView = VK_NULL_HANDLE;
    
    // present fb color attahcment (if MSAA enable)
    VkImage mColorImage = VK_NULL_HANDLE;
    VkDeviceMemory mColorImageMemory = VK_NULL_HANDLE;
    VkImageView mColorImageView = VK_NULL_HANDLE;

    // swapchain fb resources
    std::vector<VkFramebuffer> mSwapchainFramebuffers = {};
    VkRenderPass mPresentRenderPass = VK_NULL_HANDLE;

    SceneRenderBase* mSceneRender = nullptr;

    // 交互数据
    std::mutex mInputEventMutex;
    InputEventInfo mInputInfo = {};

    std::atomic<bool> mFramebufferResized = false;

};
}   // namespace framework

#endif // !__DRAW_MESH_THREAD_H__


