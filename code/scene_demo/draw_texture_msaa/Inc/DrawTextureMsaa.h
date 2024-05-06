#ifndef __DRAW_TEXTURE_MSAA_H__
#define __DRAW_TEXTURE_MSAA_H__

#include <vulkan/vulkan.h>

#include "SceneRenderBase.h"
#include "FrameworkHeaders.h"

namespace framework {
class DrawTextureMsaa : public SceneRenderBase {
public:
    DrawTextureMsaa() {}
    ~DrawTextureMsaa() {}

    void Init(const RenderInitInfo& initInfo) override;
    void CleanUp() override;
    std::vector<VkCommandBuffer>& RecordCommand(const RenderInputInfo& input) override;
    void OnResize(VkExtent2D newExtent) override;

private:

    void CreatePipelines();
    void CleanUpPipelines();

    void CreateBuffers();
    void CleanUpBuffers();

    void CreateTextures();
    void CleanUpTextures();

    void CreateTextureSampler();
    void CleanUpTextureSampler();

    void CreateDescriptorPool();
    void CleanUpDescriptorPool();
    void CreateDescriptorSets();

private:
    std::vector<VkCommandBuffer> mPrimaryCommandBuffers = {};
    VkCommandBuffer mCommandBuffer = VK_NULL_HANDLE;

    PipelineObjecs mPipeline = {};

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

    std::vector<uint16_t> mQuadIndices = {
        0, 1, 2, 2, 3, 0
    };
};
}

#endif // __DRAW_TEXTURE_MSAA_H__
