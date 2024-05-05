#ifndef __DRAW_SCENE_TEST_QUAD__
#define __DRAW_SCENE_TEST_QUAD__

#include <vulkan/vulkan.h>

#include "SceneRenderBase.h"
#include "FrameworkHeaders.h"
#include "TestMesh.h"
#include "Camera.h"

namespace framework {
class DrawSceneTest : public SceneRenderBase {
public:
    DrawSceneTest();
    ~DrawSceneTest();

    void Init(const RenderInitInfo& initInfo) override;
    void CleanUp() override;
    std::vector<VkCommandBuffer>& RecordCommand(const RenderInputInfo& input) override;
    void ProcessInputEnvent(const InputEventInfo& inputEnventInfo) override;

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

    void CreateTextures();
    void CleanUpTextures();

    void CreateTextureSampler();
    void CleanUpTextureSampler();

    void UpdataUniformBuffer(float aspectRatio);

private:
    std::vector<VkCommandBuffer> mPrimaryCommandBuffers = {};

    // ---- render objects ----
    PipelineObjecs mPipeline = {};

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

    // test texture
    VkImage mTestTextureImage = VK_NULL_HANDLE;
    VkDeviceMemory mTestTextureImageMemory = VK_NULL_HANDLE;
    VkImageView mTestTextureImageView = VK_NULL_HANDLE;
    VkSampler mTexureSampler = VK_NULL_HANDLE;

    // data
    struct UboMvpMatrix {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

    TestMesh* mMesh = nullptr;
    Camera* mCamera = nullptr;

    bool mLastLeftPress = false;
    glm::vec2 mLastCursorPose = { 0.0, 0.0 };
    std::unordered_map<int, uint16_t> mKeyPressStatus = {
        { FRAMEWORK_KEY_W, false },
        { FRAMEWORK_KEY_A, false },
        { FRAMEWORK_KEY_S, false },
        { FRAMEWORK_KEY_D, false },
        { FRAMEWORK_KEY_Q, false },
        { FRAMEWORK_KEY_E, false },
    };
    
};
}

#endif // __DRAW_SCENE_TEST_QUAD__
