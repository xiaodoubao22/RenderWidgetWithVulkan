#ifndef __DRAW_TRIANGLE_THREAD_H__
#define __DRAW_TRIANGLE_THREAD_H__

#include "Thread.h"
#include "GraphicsDevice.h"
#include "Swapchain.h"
#include "PipelineTest.h"
#include "RenderPassTest.h"

#include <vector>
#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace window {
    class WindowTemplate;
}

namespace render {
    class DrawTriangleThread : public common::Thread {
    public:
        explicit DrawTriangleThread(window::WindowTemplate& a);
        ~DrawTriangleThread();

        void SetFramebufferResized();

    private:
        virtual void OnThreadInit() override;
        virtual void OnThreadLoop() override;
        virtual void OnThreadDestroy() override;

    private:
        // ----- rener functions -----
        void RecordCommandBuffer(VkCommandBuffer commandBuffer, int32_t imageIndex);
        void UpdataUniformBuffer();
        void Resize();

        // ----- create and clean up ----- 
        void CreateInstance();

        void CreateDepthResources();
        void CleanUpDepthResources();

        void CreateFramebuffers();
        void CleanUpFramebuffers();

        void CreateSyncObjects();
        void CleanUpSyncObjects();

        void CreateVertexBuffer();
        void CleanUpVertexBuffer();

        void CreateIndexBuffer();
        void CleanUpIndexBuffer();

        void CreateUniformBuffer();
        void CleanUpUniformBuffer();

        void CreateDescriptorPool();
        void CleanUpDescriptorPool();
        void CreateDescriptorSets();

        // ----- tool functions -----
        void CheckValidationLayerSupport(bool enableValidationLayer);
        bool CheckExtensionSupport(const std::vector<const char*>& target);

    private:
        bool mEnableValidationLayer = false;
        bool mFramebufferResized = false;
        std::mutex mFramebufferResizeMutex;

        // ---- externel objects ----
        window::WindowTemplate& mWindow;

        // ---- basic objects ----
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

        // vertex buffer
        VkBuffer mVertexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory mVertexBufferMemory = VK_NULL_HANDLE;

        // index buffer
        VkBuffer mIndexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory mIndexBufferMemory = VK_NULL_HANDLE;

        // uniform buffer
        VkBuffer mUniformBuffer = VK_NULL_HANDLE;
        VkDeviceMemory mUniformBuffersMemory = VK_NULL_HANDLE;
        void* mUniformBuffersMapped = nullptr;

        VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
        VkDescriptorSet mDescriptorSet = VK_NULL_HANDLE;

    };
}

#endif // !__DRAW_TRIANGLE_THREAD_H__

