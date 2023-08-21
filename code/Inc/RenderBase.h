#ifndef __RENDER_BASE_H__
#define __RENDER_BASE_H__

#include <vector>
#include <vulkan/vulkan.h>

#include "GraphicsDevice.h"
#include "Swapchain.h"
#include "PipelineTest.h"
#include "RenderPassTest.h"

namespace window {
    class WindowTemplate;
}

namespace render {
    class RenderBase {
    public:
        explicit RenderBase(window::WindowTemplate& a);
        ~RenderBase();

        void Init(bool enableValidationLayer);
        void CleanUp();
        void Update();

    private:
        void CreateInstance();
        void CreateDepthResources();
        void CreateFramebuffers();
        void CreateSyncObjects();
        void CleanUpDepthResources();
        void CleanUpFramebuffers();
        void CleanUpSyncObjects();

        void RecordCommandBuffer(VkCommandBuffer commandBuffer, int32_t imageIndex);
        

        // ----- tool functions -----
        void CheckValidationLayerSupport(bool enableValidationLayer);
        bool CheckExtensionSupport(const std::vector<const char*>& target);


    private:
        bool mEnableValidationLayer = false;

        // externel objects
        window::WindowTemplate& mWindow;

        // basic objects
        VkInstance mInstance = VK_NULL_HANDLE;
        VkSurfaceKHR mSurface = VK_NULL_HANDLE;

        GraphicsDevice* mGraphicsDevice = nullptr;
        Swapchain* mSwapchain = nullptr;

        // ---- render objects ----
        RenderPassTest* mRenderPassTest = nullptr;
        PipelineTest* mPipelineTest = nullptr;

        // depth resources
        VkImage mDepthImage = VK_NULL_HANDLE;
        VkDeviceMemory mDepthImageMemory = VK_NULL_HANDLE;
        VkImageView mDepthImageView = VK_NULL_HANDLE;

        std::vector<VkFramebuffer> mSwapchainFramebuffers = {};
        VkCommandBuffer mCommandBuffer = VK_NULL_HANDLE;

        // sync objecs
        VkSemaphore mImageAvailableSemaphore = VK_NULL_HANDLE;
        VkSemaphore mRenderFinishedSemaphore = VK_NULL_HANDLE;
        VkFence mInFlightFence = VK_NULL_HANDLE;

    };
}


#endif // __RENDER_BASE_H__

