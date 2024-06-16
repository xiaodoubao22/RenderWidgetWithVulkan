#ifndef __DRAW_VRS_TEST_H__
#define __DRAW_VRS_TEST_H__

#include <vulkan/vulkan.h>

#include "SceneRenderBase.h"
#include "FrameworkHeaders.h"
#include "TestMesh.h"
#include "Camera.h"
#include "VmaUsage.h"

#include "VrsPipeline.h"

namespace framework {
class DrawVrsTest : public SceneRenderBase {
public:
    DrawVrsTest();
    ~DrawVrsTest();

    void Init(const RenderInitInfo& initInfo) override;
    void CleanUp() override;
    std::vector<VkCommandBuffer>& RecordCommand(const RenderInputInfo& input) override;
    void OnResize(VkExtent2D newExtent) override;
    void ProcessInputEvent(const InputEventInfo& inputEventInfo) override;
    void RequestPhysicalDeviceFeatures(PhysicalDevice* physicalDevice) override;

private:
    void CreateRenderPasses();
    void CleanUpRenderPasses();

    void CreateMainFbAttachment();
    void CleanUpMainFbAttachment();

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

    void UpdateDescriptorSets();

    // tool functions
    void RecordPresentPass(VkCommandBuffer cmdBuf, const RenderInputInfo& input);

private:
    std::vector<VkCommandBuffer> mPrimaryCommandBuffers = {};

    // ---- render objects ----
    PipelineObjecs mPipelinePresent = {};
    PipelineObjecs mPipelineDrawPbr = {};
    PipelineObjecs mPipelinePbrTexture = {};
    PipelineObjecs mPipelineBlendVrsImage = {};

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
    VkBuffer mUboGlobalMatrixVP = VK_NULL_HANDLE;
    void* mUboGlobalMatrixVPAddr = nullptr;
    VkBuffer mUboInstanceMatrixM = VK_NULL_HANDLE;
    void* mUboInstanceMatrixMAddr = nullptr;
    VkDeviceMemory mUniformBuffersMemory = VK_NULL_HANDLE;

    VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet mDescriptorSetPbr = VK_NULL_HANDLE;
    VkDescriptorSet mDescriptorSetPbrTexture = VK_NULL_HANDLE;
    VkDescriptorSet mDescriptorSetPresent = VK_NULL_HANDLE;
    VkDescriptorSet mDescriptorSetBlendVrs = VK_NULL_HANDLE;

    // test texture
    VmaAllocation RoughnessImageAllocation = VK_NULL_HANDLE;
    VkImage mRoughnessImage = VK_NULL_HANDLE;
    VkImageView mRoughnessImageView = VK_NULL_HANDLE;
    VmaAllocation mMatallicImageAllocation = VK_NULL_HANDLE;
    VkImage mMatallicImage = VK_NULL_HANDLE;
    VkImageView mMatallicImageView = VK_NULL_HANDLE;
    VmaAllocation mAlbedoImageAllocation = VK_NULL_HANDLE;
    VkImage mAlbedoImage = VK_NULL_HANDLE;
    VkImageView mAlbedoImageView = VK_NULL_HANDLE;
    VmaAllocation mNormalImageAllocation = VK_NULL_HANDLE;
    VkImage mNormalImage = VK_NULL_HANDLE;
    VkImageView mNormalImageView = VK_NULL_HANDLE;

    VkSampler mTexureSampler = VK_NULL_HANDLE;
    VkSampler mTexureSamplerNearst = VK_NULL_HANDLE;

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
    const VkFormat mMainFbColorFormat = VK_FORMAT_B10G11R11_UFLOAT_PACK32;  //VK_FORMAT_R8G8B8A8_UNORM;
    const VkFormat mMainFbDepthFormat = VK_FORMAT_D24_UNORM_S8_UINT;

    VrsPipeline* mVrsPipeline = nullptr;
    bool mBlendKeyPress = false;

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

    struct GlobalMatrixVP {
        glm::mat4 view;
        glm::mat4 proj;
        glm::vec3 cameraPos;
    };

    struct InstanceMatrixM {
        glm::mat4 model;
    };
    size_t mInstanceMatrixMAlignment = 0;
    static constexpr uint32_t INSTANCE_NUM = 5;
    uint32_t mInstanceMatrixMOffsets[INSTANCE_NUM] = {};

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

#endif // __DRAW_VRS_TEST_H__
