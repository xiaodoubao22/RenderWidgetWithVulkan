#ifndef __DRAW_ATTACHMENT_SHADING_RATE_THREAD__
#define __DRAW_ATTACHMENT_SHADING_RATE_THREAD__

#include "Thread.h"
//#include "PipelineDrawTexture.h"
#include "PipelineVariableShadingRate.h"
#include "RenderPassShadingRate.h"
#include "RenderBase.h"
#include "Mesh.h"

#include <vector>
#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace render {
    class DrawAttachmentShadingRateThread : public common::Thread, public RenderBase {
    public:
        explicit DrawAttachmentShadingRateThread(window::WindowTemplate& w);
        ~DrawAttachmentShadingRateThread();

        void SetFramebufferResized();

    private:
        virtual void OnThreadInit() override;
        virtual void OnThreadLoop() override;
        virtual void OnThreadDestroy() override;

        bool PhysicalDeviceSelectionCondition(VkPhysicalDevice physicalDevice) override;
        std::vector<const char*> FillDeviceExtensions() override;
        void RequestPhysicalDeviceFeatures(PhysicalDevice* physicalDevice) override;

    private:
        // ----- render functions -----
        void RecordCommandBuffer(VkCommandBuffer commandBuffer, int32_t imageIndex);
        void Resize();

        // ----- create and clean up ----- 
        void CreateDepthResources();
        void CleanUpDepthResources();

        void CreateShadingRateTextureResource();
        void CleanUpShadingRateTextureResource();

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

        void VkCmdSetFragmentShadingRateKHR(VkInstance instance, VkCommandBuffer commandBuffer,
            const VkExtent2D* pFragmentSize, const VkFragmentShadingRateCombinerOpKHR combinerOps[2]) {
            auto func = (PFN_vkCmdSetFragmentShadingRateKHR)vkGetInstanceProcAddr(instance, "vkCmdSetFragmentShadingRateKHR");
            if (func != nullptr) {
                func(commandBuffer, pFragmentSize, combinerOps);
                return;
            }
            else {
                std::runtime_error("vkGetInstanceProcAddr vkCmdSetFragmentShadingRateKHR failed!");
                return;
            }
        }

        VkResult GetPhysicalDeviceFragmentShadingRatesKHR(VkInstance instance, VkPhysicalDevice physicalDevice,
            uint32_t* pFragmentShadingRateCount, VkPhysicalDeviceFragmentShadingRateKHR* pFragmentShadingRates) {
            auto func = (PFN_vkGetPhysicalDeviceFragmentShadingRatesKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceFragmentShadingRatesKHR");
            if (func != nullptr) {
                return func(physicalDevice, pFragmentShadingRateCount, pFragmentShadingRates);
            }
            else {
                return VK_ERROR_EXTENSION_NOT_PRESENT;
            }
        }
    private:
        bool mFramebufferResized = false;
        std::mutex mFramebufferResizeMutex;

        // ---- render objects ----
        RenderPassShadingRate* mRenderPassShadingRate = nullptr;
        PipelineVariableShadingRate* mPipelineVariableShadingRate = nullptr;

        // depth resources
        VkImage mDepthImage = VK_NULL_HANDLE;
        VkDeviceMemory mDepthImageMemory = VK_NULL_HANDLE;
        VkImageView mDepthImageView = VK_NULL_HANDLE;

        // shading rate resource
        VkImage mShadingRateImage = VK_NULL_HANDLE;
        VkDeviceMemory mShadingRateImageMemory = VK_NULL_HANDLE;
        VkImageView mShadingRateImageView = VK_NULL_HANDLE;

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

        // -------- data ------------
        std::vector<Vertex2DColorTexture> mQuadVertices = {
            {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},     // 左上
            {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},      // 右上
            {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},       // 右下
            {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}       // 左下
        };

        const std::vector<uint16_t> mQuadIndices = {
            0, 1, 2, 2, 3, 0
        };
    };
}


#endif // !__DRAW_ATTACHMENT_SHADING_RATE_THREAD__


