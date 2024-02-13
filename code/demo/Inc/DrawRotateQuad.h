#ifndef __DRAW_ROTATE_QUAD__
#define __DRAW_ROTATE_QUAD__

#include <vulkan/vulkan.h>

#include "SceneRenderBase.h"
#include "frameworkHeaders.h"
#include "TestMesh.h"

namespace render {
class DrawRotateQuad : public SceneRenderBase {
public:
    DrawRotateQuad();
    ~DrawRotateQuad();

    virtual void Init(const RenderInitInfo& initInfo) override;
    virtual void CleanUp() override;
    virtual std::vector<VkCommandBuffer>& RecordCommand(const RenderInputInfo& input) override;

    virtual void GetRequiredDeviceExtensions(std::vector<const char*>& deviceExt) override;
    virtual void GetRequiredInstanceExtensions(std::vector<const char*>& deviceExt) override;

private:
    void CreateRenderPasses();
    void CleanUpRenderPasses();

    void CreateVertexBuffer();
    void CleanUpVertexBuffer();

    void CreateIndexBuffer();
    void CleanUpIndexBuffer();

    void CreateUniformBuffer();
    void CleanUpUniformBuffer();

    void CreateDescriptorPool();
    void CleanUpDescriptorPool();
    void CreateDescriptorSets();

    void CreatePipelines();
    void CleanUpPipelines();

    void UpdataUniformBuffer(float aspectRatio);

private:
    std::vector<VkCommandBuffer> mPrimaryCommandBuffers = {};

    // ---- render objects ----
    PipelineComponents mPipeline = {};

    VkCommandBuffer mCommandBuffer = VK_NULL_HANDLE;

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

    // data
    struct UboMvpMatrix {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

    std::vector<Vertex2DColor> mTriangleVertices = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
    };

    const std::vector<uint16_t> mTriangleIndices = {
        0, 1, 2, 2, 3, 0
    };
};
}

#endif // __DRAW_ROTATE_QUAD__
