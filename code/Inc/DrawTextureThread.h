#ifndef __DRAW_TEXTURE_THREAD__
#define __DRAW_TEXTURE_THREAD__

#include "Thread.h"
#include "GraphicsDevice.h"
#include "Swapchain.h"
#include "PipelineDrawTexture.h"
#include "RenderPassTest.h"
#include "RenderBase.h"

#include <vector>
#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace render {
    class DrawTextureThread : public common::Thread, public RenderBase {
    public:
        explicit DrawTextureThread(window::WindowTemplate& w);
        ~DrawTextureThread();

        void SetFramebufferResized();

    private:
        virtual void OnThreadInit() override;
        virtual void OnThreadLoop() override;
        virtual void OnThreadDestroy() override;

    private:
        // ----- rener functions -----
        void RecordCommandBuffer(VkCommandBuffer commandBuffer, int32_t imageIndex);
        void Resize();

        // ----- create and clean up ----- 
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

        void CreateTexture();
        void CleanUpTexture();

        void CreateTextureSampler();
        void CleanUpTextureSampler();

        void CreateDescriptorPool();
        void CleanUpDescriptorPool();
        void CreateDescriptorSets();

    private:
        bool mFramebufferResized = false;
        std::mutex mFramebufferResizeMutex;

        // ---- render objects ----
        RenderPassTest* mRenderPassTest = nullptr;
        PipelineDrawTexture* mPipelineDrawTexture = nullptr;

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

        // test texture
        VkImage mTestTextureImage = VK_NULL_HANDLE;
        VkDeviceMemory mTestTextureImageMemory = VK_NULL_HANDLE;
        VkImageView mTestTextureImageView = VK_NULL_HANDLE;
        VkSampler mTexureSampler = VK_NULL_HANDLE;

        // descriptors
        VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
        VkDescriptorSet mDescriptorSet = VK_NULL_HANDLE;
    };
}


#endif // !__DRAW_TEXTURE_THREAD__

