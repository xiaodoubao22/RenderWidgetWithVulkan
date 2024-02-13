#ifndef __DRAW_TEXTURE_THREAD__
#define __DRAW_TEXTURE_THREAD__

#include "Thread.h"
#include "RenderBase.h"
#include "TestMesh.h"
#include "GraphicsPipelineConfig.h"

#include <vector>
#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace render {
class DrawTextureThread : public Thread, public RenderBase {
public:
    explicit DrawTextureThread(window::WindowTemplate& w);
    ~DrawTextureThread();

private:
    virtual void OnThreadInit() override;
    virtual void OnThreadLoop() override;
    virtual void OnThreadDestroy() override;

private:
    // ----- render functions -----
    void RecordCommandBuffer(VkCommandBuffer commandBuffer, int32_t imageIndex);
    void Resize();

    // ----- create and clean up ----- 
    void CreateColorResources();
    void CleanUpColorResources();

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

    void CreateRenderPasses();
    void CleanUpRenderPasses();

    void CreatePipelines();
    void CleanUpPipelines();

private:
    // ---- render objects ----
    VkFormat mMainDepthFormat = VK_FORMAT_UNDEFINED;
    VkRenderPass mMainRenderPass = VK_NULL_HANDLE;
    PipelineComponents mPipeline = {};

    // depth resources
    VkImage mDepthImage = VK_NULL_HANDLE;
    VkDeviceMemory mDepthImageMemory = VK_NULL_HANDLE;
    VkImageView mDepthImageView = VK_NULL_HANDLE;

    // msaa color resources
    VkImage mMsaaColorImage = VK_NULL_HANDLE;
    VkDeviceMemory mMsaaColorImageMemory = VK_NULL_HANDLE;
    VkImageView mMsaaColorImageView = VK_NULL_HANDLE;

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
        {{-0.3f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},     // 左上
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},      // 右上
        {{0.3f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},       // 右下
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}       // 左下
    };

    const std::vector<uint16_t> mQuadIndices = {
        0, 1, 2, 2, 3, 0
    };

    VkSampleCountFlagBits mMsaaSamples = VK_SAMPLE_COUNT_4_BIT;
};
}


#endif // !__DRAW_TEXTURE_THREAD__

