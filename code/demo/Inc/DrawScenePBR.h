#ifndef __DRAW_SCENE_PBR_H__
#define __DRAW_SCENE_PBR_H__

#include <vulkan/vulkan.h>

#include "SceneRenderBase.h"
#include "frameworkHeaders.h"
#include "TestMesh.h"
#include "Camera.h"

namespace render {
class DrawScenePbr : public SceneRenderBase {
public:
    DrawScenePbr();
    ~DrawScenePbr();

    virtual void Init(const RenderInitInfo& initInfo) override;
    virtual void CleanUp() override;
    virtual std::vector<VkCommandBuffer>& RecordCommand(const RenderInputInfo& input) override;
    virtual void OnResize(VkExtent2D newExtent) override;

    virtual void GetRequiredDeviceExtensions(std::vector<const char*>& deviceExt) override;
    virtual void GetRequiredInstanceExtensions(std::vector<const char*>& deviceExt) override;

    virtual void ProcessInputEnvent(const InputEventInfo& inputEnventInfo) override;

private:
    void CreateRenderPasses();
    void CleanUpRenderPasses();

    void CreateMainFramebuffer();
    void CleanUpMainFramebuffer();

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

    // tool functions
    PipelineComponents CreatePipeline(const GraphicsPipelineConfigBase& configInfo,
        std::vector<SpvFilePath>& shaderFilePaths,
        std::vector<VkDescriptorSetLayoutBinding>& layoutBindings,
        std::vector<VkPushConstantRange>& pushConstantRanges);
    void DestroyPipeline(PipelineComponents& pipeline);

    void RecordPresentPass(VkCommandBuffer cmdBuf, const RenderInputInfo& input);

private:
    std::vector<VkCommandBuffer> mPrimaryCommandBuffers = {};

    // ---- render objects ----
    PipelineComponents mPipelinePresent = {};
    PipelineComponents mPipelineDrawPbr = {};

    VkCommandBuffer mCommandBuffer = VK_NULL_HANDLE;

    // vertex buffer
    VkBuffer mVertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory mVertexBufferMemory = VK_NULL_HANDLE;

    // index buffer
    VkBuffer mIndexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory mIndexBufferMemory = VK_NULL_HANDLE;

    // uniform buffer
    VkBuffer mUboMvp = VK_NULL_HANDLE;
    void* mUboMvpMapped = nullptr;
    VkBuffer mUboMaterial = VK_NULL_HANDLE;
    void* mUboMaterialMapped = nullptr;
    VkDeviceMemory mUniformBuffersMemory = VK_NULL_HANDLE;

    VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet mDescriptorSetPbr = VK_NULL_HANDLE;
    VkDescriptorSet mDescriptorSetPresent = VK_NULL_HANDLE;

    // test texture
    VkImage mTestTextureImage = VK_NULL_HANDLE;
    VkDeviceMemory mTestTextureImageMemory = VK_NULL_HANDLE;
    VkImageView mTestTextureImageView = VK_NULL_HANDLE;
    VkSampler mTexureSampler = VK_NULL_HANDLE;

    // main fb resources
    VkImage mMainFbColorImage = VK_NULL_HANDLE;
    VkImageView mMainFbColorImageView = VK_NULL_HANDLE;
    VkImage mMainFbDepthImage = VK_NULL_HANDLE;
    VkImageView mMainFbDepthImageView = VK_NULL_HANDLE;
    VkDeviceMemory mMainFbMemory = VK_NULL_HANDLE;
    VkFramebuffer mMainFrameBuffer = VK_NULL_HANDLE;
    VkRenderPass mMainPass = VK_NULL_HANDLE;

    const float mResolutionFactor = 0.8;
    VkExtent2D mMainFbExtent = {};
    const VkFormat mMainFbColorFormat = VK_FORMAT_B10G11R11_UFLOAT_PACK32;
    const VkFormat mMainFbDepthFormat = VK_FORMAT_D24_UNORM_S8_UINT;

    // data
    struct UboMvpMatrix {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
        glm::vec3 cameraPos;
    };

    struct UniformMaterial {
        float roughness;
        float metallic;
        alignas(16)glm::vec3 albedo;
        alignas(16)glm::vec3 modelOffset;
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

#endif // __DRAW_SCENE_PBR_H__
