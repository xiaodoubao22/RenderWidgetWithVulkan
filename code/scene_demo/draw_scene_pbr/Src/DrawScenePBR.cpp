#include "DrawScenePbr.h"

#include <stdexcept>
#include <array>
#include <chrono>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Log.h"
#include "SceneDemoDefs.h"

namespace framework {
DrawScenePbr::DrawScenePbr()
{
    mMesh = new TestMesh;
    mCamera = new Camera;
}

DrawScenePbr::~DrawScenePbr()
{
    delete mCamera;
    delete mMesh;
}

void DrawScenePbr::Init(const RenderInitInfo& initInfo)
{
    if (!SceneRenderBase::InitCheck(initInfo)) {
        return;
    }

    mMainFbExtent = initInfo.swapchainExtent;
    mMainFbExtent.width *= mResolutionFactor;
    mMainFbExtent.height *= mResolutionFactor;

    mCamera->mTargetDistance = 20.0f;
    mCamera->mTargetPoint = glm::vec3(0.0, 5.0, 5.0);
    mMesh->GenerateSphere(1.0f, glm::vec3(0.0), glm::uvec2(64, 64));

    CreateRenderPasses();
    CreateMainFramebuffer();
    CreatePipelines();
    mCommandBuffer = mDevice->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    CreateVertexBuffer();
    CreateIndexBuffer();
    CreateUniformBuffer();
    CreateTextures();
    CreateTextureSampler();
    CreateDescriptorPool();
    CreateDescriptorSets();
}

void DrawScenePbr::CleanUp()
{
    CleanUpDescriptorPool();
    CleanUpTextureSampler();
    CleanUpTextures();
    CleanUpUniformBuffer();
    CleanUpIndexBuffer();
    CleanUpVertexBuffer();
    mDevice->FreeCommandBuffer(mCommandBuffer);
    CleanUpPipelines();
    CleanUpMainFramebuffer();
    CleanUpRenderPasses();
}

std::vector<VkCommandBuffer>& DrawScenePbr::RecordCommand(const RenderInputInfo& input)
{
    // 更新uniform buffer
    float aspectRatio = (float)input.swapchainExtent.width / (float)input.swapchainExtent.height;
    UpdataUniformBuffer(aspectRatio);

    vkResetCommandBuffer(mCommandBuffer, 0);
    // 开始写入
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;
    if (vkBeginCommandBuffer(mCommandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("fiaile to begin recording command buffer!");
    }

    std::vector<VkClearValue> clearValuesMain = { { 0.1f, 0.1f, 0.1f, 1.0f }, consts::CLEAR_DEPTH_ONE_STENCIL_ZERO };
    VkRect2D renderArea = { {0, 0}, {mMainFbExtent.width, mMainFbExtent.height} };
    VkRenderPassBeginInfo renderPassInfoMain = vulkanInitializers::RenderPassBeginInfo(
        mMainPass, mMainFrameBuffer, renderArea, clearValuesMain);
    vkCmdBeginRenderPass(mCommandBuffer, &renderPassInfoMain, VK_SUBPASS_CONTENTS_INLINE);

    // 绑定Pipeline
    vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineDrawPbr.pipeline);
    VkViewport viewportMain = { 0.0f, 0.0f, mMainFbExtent.width, mMainFbExtent.height, 0.0f, 1.0f };
    vkCmdSetViewport(mCommandBuffer, 0, 1, &viewportMain);
    vkCmdSetScissor(mCommandBuffer, 0, 1, &renderArea);

    // 绑定顶点缓冲
    std::vector<VkBuffer> vertexBuffersMain = { mVertexBuffer };
    std::vector<VkDeviceSize> offsetsMain = { 0 };
    vkCmdBindVertexBuffers(mCommandBuffer, 0, 1, vertexBuffersMain.data(), offsetsMain.data());

    // 绑定索引缓冲
    vkCmdBindIndexBuffer(mCommandBuffer, mIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

    // 绑定DescriptorSet
    vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        mPipelineDrawPbr.layout,
        0, 1, &mDescriptorSetPbr,
        0, nullptr);

    UniformMaterial uboMaterial{};
    uboMaterial.albedo = glm::vec3(1.0, 0.5, 0.0);

    int ySegMent = 5;
    int zSegMent = 5;
    float SphereDistance = 2.5f;
    for (int y = 0; y < ySegMent; y++) {
        for (int z = 0; z < zSegMent; z++) {
            uboMaterial.roughness = 0.2f + static_cast<float>(z) / zSegMent;
            uboMaterial.metallic = 0.2f + static_cast<float>(y) / ySegMent;
            uboMaterial.modelOffset = glm::vec3(0.0, SphereDistance * y, SphereDistance * z);
            vkCmdPushConstants(mCommandBuffer, mPipelineDrawPbr.layout, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(UniformMaterial), &uboMaterial);
            vkCmdDrawIndexed(mCommandBuffer, mMesh->GetIndexData().size(), 1, 0, 0, 0);
        }
    }

    vkCmdEndRenderPass(mCommandBuffer);

    // =============================================================================

    Device::ImageMemoryBarrierInfo imageBarrierInfo{};
    imageBarrierInfo.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imageBarrierInfo.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageBarrierInfo.srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    imageBarrierInfo.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    imageBarrierInfo.dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    imageBarrierInfo.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    mDevice->AddCmdPipelineBarrier(mCommandBuffer, mMainFbColorImage, VK_IMAGE_ASPECT_COLOR_BIT, imageBarrierInfo);

    // =============================================================================

    RecordPresentPass(mCommandBuffer, input);

    // 写入完成
    if (vkEndCommandBuffer(mCommandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }

    mPrimaryCommandBuffers.clear();
    mPrimaryCommandBuffers.emplace_back(mCommandBuffer);
    return mPrimaryCommandBuffers;
}

void DrawScenePbr::OnResize(VkExtent2D newExtent)
{
    if (newExtent.width == 0 || newExtent.height == 0) {
        return;
    }
    mMainFbExtent = newExtent;
    mMainFbExtent.width *= mResolutionFactor;
    mMainFbExtent.height *= mResolutionFactor;

    CleanUpMainFramebuffer();
    CreateMainFramebuffer();

    UpdateDescriptorSets();
}

void DrawScenePbr::GetRequiredDeviceExtensions(std::vector<const char*>& deviceExt)
{
    return;
}

void DrawScenePbr::GetRequiredInstanceExtensions(std::vector<const char*>& deviceExt)
{
    return;
}

void DrawScenePbr::ProcessInputEnvent(const InputEventInfo& inputEnventInfo)
{
    // mouse inpute
    glm::vec2 curCursorPose = glm::vec2(inputEnventInfo.cursorX, inputEnventInfo.cursorY);
    if (inputEnventInfo.leftPressFlag && mLastLeftPress) {
        mCamera->ProcessRotate(curCursorPose - mLastCursorPose);
    }
    mLastLeftPress = inputEnventInfo.leftPressFlag;
    mLastCursorPose = curCursorPose;

    // key input
    if (inputEnventInfo.keyAction == FRAMEWORK_KEY_PRESS || inputEnventInfo.keyAction == FRAMEWORK_KEY_RELEASE) {
        if (mKeyPressStatus.find(inputEnventInfo.key) != mKeyPressStatus.end()) {
            mKeyPressStatus[inputEnventInfo.key] = static_cast<uint16_t>(inputEnventInfo.keyAction);
        }
    }
    bool directionKeyPress = false;
    for (auto it = mKeyPressStatus.begin(); it != mKeyPressStatus.end(); it++) {
        if (it->second) {
            directionKeyPress = true;
            break;
        }
    }
    if (directionKeyPress) {
        float dxFront = mKeyPressStatus[FRAMEWORK_KEY_W] * 0.1f - mKeyPressStatus[FRAMEWORK_KEY_S] * 0.1f;
        float dxRight = mKeyPressStatus[FRAMEWORK_KEY_D] * 5.0f - mKeyPressStatus[FRAMEWORK_KEY_A] * 5.0f;
        float dxUp = mKeyPressStatus[FRAMEWORK_KEY_E] * 5.0f - mKeyPressStatus[FRAMEWORK_KEY_Q] * 5.0f;
        mCamera->ProcessMove(glm::vec3(dxRight, dxUp, dxFront));
    }

    mCamera->UpdateView();
}

void DrawScenePbr::CreateRenderPasses()
{
    // subpass
    VkAttachmentReference2 colorAttachmentRef =
        vulkanInitializers::AttachmentReference2(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkAttachmentReference2 depthAttachmentRef =
        vulkanInitializers::AttachmentReference2(1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    std::vector<VkSubpassDescription2> subpasses = {
        vulkanInitializers::SubpassDescription2(VK_PIPELINE_BIND_POINT_GRAPHICS, &colorAttachmentRef, &depthAttachmentRef),
    };

    std::vector<VkSubpassDependency2> dependencys = { vulkanInitializers::SubpassDependency2(VK_SUBPASS_EXTERNAL, 0) };
    dependencys[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencys[0].srcAccessMask = 0;
    dependencys[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencys[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::vector<VkAttachmentDescription2> mAttachments2(2);
    // 颜色附件
    mAttachments2[0] = vulkanInitializers::AttachmentDescription2(mMainFbColorFormat);
    vulkanInitializers::AttachmentDescription2SetOp(mAttachments2[0],
        VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
    vulkanInitializers::AttachmentDescription2SetLayout(mAttachments2[0],
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    // 深度附件
    mAttachments2[1] = vulkanInitializers::AttachmentDescription2(mMainFbDepthFormat);
    vulkanInitializers::AttachmentDescription2SetOp(mAttachments2[1],
        VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE);
    vulkanInitializers::AttachmentDescription2SetLayout(mAttachments2[1],
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    VkRenderPassCreateInfo2 renderPassInfo = vulkanInitializers::RenderPassCreateInfo2(mAttachments2, subpasses);
    vulkanInitializers::RenderPassCreateInfo2SetArray(renderPassInfo, dependencys);
    if (vkCreateRenderPass2(mDevice->Get(), &renderPassInfo, nullptr, &mMainPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void DrawScenePbr::CleanUpRenderPasses()
{
    vkDestroyRenderPass(mDevice->Get(), mMainPass, nullptr);
}

void DrawScenePbr::CreateMainFramebuffer()
{
    // create images
    VkImageCreateInfo colorImageInfo = vulkanInitializers::ImageCreateInfo(
        VK_IMAGE_TYPE_2D, mMainFbColorFormat,
        { mMainFbExtent.width, mMainFbExtent.height, 1 },
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    if (vkCreateImage(mDevice->Get(), &colorImageInfo, nullptr, &mMainFbColorImage) != VK_SUCCESS) {    // 创建VkImage
        throw std::runtime_error("failed to mMainFbColorImage!");
    }

    VkImageCreateInfo depthImageInfo = vulkanInitializers::ImageCreateInfo(
        VK_IMAGE_TYPE_2D, mMainFbDepthFormat,
        { mMainFbExtent.width, mMainFbExtent.height, 1 },
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    if (vkCreateImage(mDevice->Get(), &depthImageInfo, nullptr, &mMainFbDepthImage) != VK_SUCCESS) {    // 创建VkImage
        throw std::runtime_error("failed to mMainFbDepthImage!");
    }

    // allocate memory
    VkMemoryRequirements colorMemRequirements{};
    VkMemoryRequirements depthMemRequirements{};
    vkGetImageMemoryRequirements(mDevice->Get(), mMainFbColorImage, &colorMemRequirements);
    vkGetImageMemoryRequirements(mDevice->Get(), mMainFbDepthImage, &depthMemRequirements);
    VkDeviceSize colorMemSize =
        static_cast<VkDeviceSize>(std::ceil(static_cast<double>(colorMemRequirements.size) / depthMemRequirements.alignment)) *
        depthMemRequirements.alignment;
    VkDeviceSize depthMemSize = depthMemRequirements.size;

    VkMemoryAllocateInfo allocInfo = vulkanInitializers::MemoryAllocateInfo();
    allocInfo.allocationSize = colorMemSize + depthMemSize;
    allocInfo.memoryTypeIndex = mDevice->GetPhysicalDevice()->FindMemoryType(
        colorMemRequirements.memoryTypeBits & depthMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (vkAllocateMemory(mDevice->Get(), &allocInfo, nullptr, &mMainFbMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate mMainFbMemory");
    }

    // bind memory
    vkBindImageMemory(mDevice->Get(), mMainFbColorImage, mMainFbMemory, 0);
    vkBindImageMemory(mDevice->Get(), mMainFbDepthImage, mMainFbMemory, colorMemSize);

    // image view
    VkImageViewCreateInfo colorImageViewInfo = vulkanInitializers::ImageViewCreateInfo(mMainFbColorImage,
        VK_IMAGE_VIEW_TYPE_2D, mMainFbColorFormat, { VK_IMAGE_ASPECT_COLOR_BIT , 0, 1, 0, 1 });
    if (vkCreateImageView(mDevice->Get(), &colorImageViewInfo, nullptr, &mMainFbColorImageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create mMainFbColorImageView!");
    }
    VkImageViewCreateInfo depthImageViewInfo = vulkanInitializers::ImageViewCreateInfo(mMainFbDepthImage,
        VK_IMAGE_VIEW_TYPE_2D, mMainFbDepthFormat,
        { VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT , 0, 1, 0, 1 });
    if (vkCreateImageView(mDevice->Get(), &depthImageViewInfo, nullptr, &mMainFbDepthImageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create mMainFbDepthImageView!");
    }

    std::vector<VkImageView> attachments = { mMainFbColorImageView, mMainFbDepthImageView };
    VkFramebufferCreateInfo framebufferInfo = vulkanInitializers::FramebufferCreateInfo(
        mMainPass, attachments, mMainFbExtent.width, mMainFbExtent.height);
    if (vkCreateFramebuffer(mDevice->Get(), &framebufferInfo, nullptr, &mMainFrameBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create frambuffer!");
    }

    LOGI("create main fb success %d", mMainFrameBuffer);
}

void DrawScenePbr::CleanUpMainFramebuffer()
{
    LOGI("clean up main fb %d", mMainFrameBuffer);
    vkDestroyFramebuffer(mDevice->Get(), mMainFrameBuffer, nullptr);
    vkDestroyImageView(mDevice->Get(), mMainFbDepthImageView, nullptr);
    vkDestroyImageView(mDevice->Get(), mMainFbColorImageView, nullptr);
    vkDestroyImage(mDevice->Get(), mMainFbDepthImage, nullptr);
    vkDestroyImage(mDevice->Get(), mMainFbColorImage, nullptr);
    vkFreeMemory(mDevice->Get(), mMainFbMemory, nullptr);
}

void DrawScenePbr::CreateVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(Vertex3D) * mMesh->GetVertexData().size();

    // 创建临时缓冲
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    mDevice->CreateBuffer(bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,		// 用途：transfer src
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);

    // 数据拷贝到临时缓冲
    void* data;
    vkMapMemory(mDevice->Get(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, mMesh->GetVertexData().data(), (size_t)bufferSize);
    vkUnmapMemory(mDevice->Get(), stagingBufferMemory);

    // 创建 mVertexBuffer
    mDevice->CreateBuffer(bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,	// 用途：transfer src + 顶点缓冲
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        mVertexBuffer, mVertexBufferMemory);

    // 复制 stagingBuffer -> mVertexBuffer
    mDevice->CopyBuffer(stagingBuffer, mVertexBuffer, bufferSize);

    // 清理临时缓冲
    vkDestroyBuffer(mDevice->Get(), stagingBuffer, nullptr);
    vkFreeMemory(mDevice->Get(), stagingBufferMemory, nullptr);
}

void DrawScenePbr::CleanUpVertexBuffer() {
    // 销毁顶点缓冲区及显存
    vkDestroyBuffer(mDevice->Get(), mVertexBuffer, nullptr);
    vkFreeMemory(mDevice->Get(), mVertexBufferMemory, nullptr);
}

void DrawScenePbr::CreateIndexBuffer() {
    VkDeviceSize bufferSize = sizeof(uint16_t) * mMesh->GetIndexData().size();

    // 创建临时缓冲
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    mDevice->CreateBuffer(bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);

    // 数据拷贝到临时缓冲
    void* data;
    vkMapMemory(mDevice->Get(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, mMesh->GetIndexData().data(), (size_t)bufferSize);
    vkUnmapMemory(mDevice->Get(), stagingBufferMemory);

    // 创建索引缓冲
    mDevice->CreateBuffer(bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        mIndexBuffer, mIndexBufferMemory);

    // 复制 stagingBuffer -> mIndexBuffer
    mDevice->CopyBuffer(stagingBuffer, mIndexBuffer, bufferSize);

    // 清理
    vkDestroyBuffer(mDevice->Get(), stagingBuffer, nullptr);
    vkFreeMemory(mDevice->Get(), stagingBufferMemory, nullptr);
}

void DrawScenePbr::CleanUpIndexBuffer() {
    vkDestroyBuffer(mDevice->Get(), mIndexBuffer, nullptr);
    vkFreeMemory(mDevice->Get(), mIndexBufferMemory, nullptr);
}

void DrawScenePbr::CreateUniformBuffer() {
    VkBufferCreateInfo bufferInfo = vulkanInitializers::BufferCreateInfo(sizeof(UboMvpMatrix),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    if (vkCreateBuffer(mDevice->Get(), &bufferInfo, nullptr, &mUboMvp) != VK_SUCCESS) {
        throw std::runtime_error("failed to create uMvp buffer");
    }

    bufferInfo.size = sizeof(UniformMaterial);
    if (vkCreateBuffer(mDevice->Get(), &bufferInfo, nullptr, &mUboMaterial) != VK_SUCCESS) {
        throw std::runtime_error("failed to create uMaterial buffer");
    }

    VkMemoryRequirements memReqMvp{};
    vkGetBufferMemoryRequirements(mDevice->Get(), mUboMvp, &memReqMvp);

    VkMemoryRequirements memReqMaterial{};
    vkGetBufferMemoryRequirements(mDevice->Get(), mUboMaterial, &memReqMaterial);

    VkDeviceSize uMvpMemSize =
        static_cast<VkDeviceSize>(std::ceil(static_cast<double>(memReqMvp.size) / memReqMaterial.alignment)) * memReqMaterial.alignment;
    VkDeviceSize uMaterialMemSize = memReqMaterial.size;

    // 申请显存
    VkMemoryAllocateInfo allocInfo = vulkanInitializers::MemoryAllocateInfo();
    allocInfo.allocationSize = uMvpMemSize + uMaterialMemSize;
    allocInfo.memoryTypeIndex = mDevice->GetPhysicalDevice()->FindMemoryType(
        memReqMvp.memoryTypeBits & memReqMaterial.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (vkAllocateMemory(mDevice->Get(), &allocInfo, nullptr, &mUniformBuffersMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    }

    // 显存绑定到缓冲区
    vkBindBufferMemory(mDevice->Get(), mUboMvp, mUniformBuffersMemory, 0);
    vkBindBufferMemory(mDevice->Get(), mUboMaterial, mUniformBuffersMemory, uMvpMemSize);

    vkMapMemory(mDevice->Get(), mUniformBuffersMemory, 0, allocInfo.allocationSize, 0, &mUboMvpMapped);
    mUboMaterialMapped = static_cast<void*>(static_cast<uint8_t*>(mUboMvpMapped) + uMvpMemSize);
}

void DrawScenePbr::CleanUpUniformBuffer() {
    vkDestroyBuffer(mDevice->Get(), mUboMvp, nullptr);
    vkDestroyBuffer(mDevice->Get(), mUboMaterial, nullptr);
    vkFreeMemory(mDevice->Get(), mUniformBuffersMemory, nullptr);
}

void DrawScenePbr::CreateDescriptorPool() {
    std::vector<VkDescriptorPoolSize> poolSizes = {};   // 池中各种类型的Descriptor个数
    poolSizes.insert(poolSizes.end(), mPipelinePresent.descriptorSizes.begin(), mPipelinePresent.descriptorSizes.end());
    poolSizes.insert(poolSizes.end(), mPipelineDrawPbr.descriptorSizes.begin(), mPipelineDrawPbr.descriptorSizes.end());

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 2;   // 池中最大能申请descriptorSet的个数
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    if (vkCreateDescriptorPool(mDevice->Get(), &poolInfo, nullptr, &mDescriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void DrawScenePbr::CleanUpDescriptorPool() {
    vkDestroyDescriptorPool(mDevice->Get(), mDescriptorPool, nullptr);
}

void DrawScenePbr::CreateDescriptorSets() {
    // present
    // 从池中申请descriptor set
    VkDescriptorSetAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    allocInfo.descriptorPool = mDescriptorPool;		// 从这个池中申请
    allocInfo.descriptorSetCount = mPipelinePresent.descriptorSetLayouts.size();
    allocInfo.pSetLayouts = mPipelinePresent.descriptorSetLayouts.data();
    if (vkAllocateDescriptorSets(mDevice->Get(), &allocInfo, &mDescriptorSetPresent) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate mDescriptorSetPresent!");
    }

    // 向descriptor set写入信息
    VkDescriptorImageInfo sampleMainFbColorImageInfo = {
        mTexureSampler, mMainFbColorImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };
    std::vector<VkWriteDescriptorSet> presentDescriptorWrites(1);
    presentDescriptorWrites[0] = vulkanInitializers::WriteDescriptorSet(mDescriptorSetPresent,
        0, 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &sampleMainFbColorImageInfo);
    vkUpdateDescriptorSets(mDevice->Get(), presentDescriptorWrites.size(), presentDescriptorWrites.data(), 0, nullptr);

    // pbr
    // 从池中申请descriptor set
    allocInfo.descriptorSetCount = mPipelineDrawPbr.descriptorSetLayouts.size();
    allocInfo.pSetLayouts = mPipelineDrawPbr.descriptorSetLayouts.data();
    if (vkAllocateDescriptorSets(mDevice->Get(), &allocInfo, &mDescriptorSetPbr) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    // 向descriptor set写入信息
    VkDescriptorBufferInfo uboMvpInfo = { mUboMvp, 0, sizeof(UboMvpMatrix) };
    VkDescriptorBufferInfo uboMaterialInfo = { mUboMaterial, 0, sizeof(UniformMaterial) };

    std::vector<VkWriteDescriptorSet> descriptorWrites(2);
    descriptorWrites[0] = vulkanInitializers::WriteDescriptorSet(mDescriptorSetPbr,
        0, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &uboMvpInfo);
    descriptorWrites[1] = vulkanInitializers::WriteDescriptorSet(mDescriptorSetPbr,
        1, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &uboMaterialInfo);
    vkUpdateDescriptorSets(mDevice->Get(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

void DrawScenePbr::CreatePipelines()
{

    PipelineFactory& pipelineFactory = PipelineFactory::GetInstance();
    pipelineFactory.SetDevice(mDevice->Get());

    std::vector<ShaderFileInfo> presentShaderFilePaths = {
        { GetConfig().directory.dirSpvFiles + std::string("ScreenQuad.vert.spv"), VK_SHADER_STAGE_VERTEX_BIT },
        { GetConfig().directory.dirSpvFiles + std::string("ScreenQuad.frag.spv"), VK_SHADER_STAGE_FRAGMENT_BIT },
    };

    std::vector<VkDescriptorSetLayoutBinding> presentLayoutBindings = {
        vulkanInitializers::DescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT),
    };

    std::vector<VkPushConstantRange> nullPushConstantRanges = {};

    GraphicsPipelineConfigInfo presentConfigInfo{};
    presentConfigInfo.SetRenderPass(mPresentRenderPass);
    presentConfigInfo.SetInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);

    mPipelinePresent = pipelineFactory.CreateGraphicsPipeline(presentConfigInfo, presentShaderFilePaths, presentLayoutBindings, nullPushConstantRanges);

    std::vector<ShaderFileInfo> pbrShaderFilePaths = {
        { GetConfig().directory.dirSpvFiles + std::string("DrawMesh.vert.spv"), VK_SHADER_STAGE_VERTEX_BIT},
        { GetConfig().directory.dirSpvFiles + std::string("DrawMeshGlossy.frag.spv"), VK_SHADER_STAGE_FRAGMENT_BIT },
    };

    std::vector<VkDescriptorSetLayoutBinding> pbrLayoutBindings = {
        vulkanInitializers::DescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT),
        vulkanInitializers::DescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT),
    };

    std::vector<VkPushConstantRange> pbrPushConstantRanges = {
        { VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(UniformMaterial) },
    };

    GraphicsPipelineConfigInfo pipelinePbrConfigInfo{};
    pipelinePbrConfigInfo.SetRenderPass(mMainPass);
    pipelinePbrConfigInfo.SetVertexInputBindings({ Vertex3D::GetBindingDescription() });
    pipelinePbrConfigInfo.SetVertexInputAttributes(Vertex3D::getAttributeDescriptions());
    pipelinePbrConfigInfo.mDepthStencilState.depthTestEnable = VK_TRUE;
    pipelinePbrConfigInfo.mDepthStencilState.depthWriteEnable = VK_TRUE;
    pipelinePbrConfigInfo.mDepthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
    pipelinePbrConfigInfo.mDepthStencilState.depthBoundsTestEnable = VK_FALSE;

    mPipelineDrawPbr = pipelineFactory.CreateGraphicsPipeline(pipelinePbrConfigInfo, pbrShaderFilePaths, pbrLayoutBindings, pbrPushConstantRanges);
}

void DrawScenePbr::CleanUpPipelines()
{
    PipelineFactory& pipelineFactory = PipelineFactory::GetInstance();
    pipelineFactory.SetDevice(mDevice->Get());

    pipelineFactory.DestroyPipelineObjecst(mPipelineDrawPbr);
    pipelineFactory.DestroyPipelineObjecst(mPipelinePresent);
}

void DrawScenePbr::CreateTextures()
{
    // 读取图片
    int texWidth, texHeight, texChannels;
    //std::string texturePath("../resource/textures/viking_room.png");
    std::string texturePath("../resource/pbr_textures/rustediron1-alt2-Unreal-Engine/rustediron2_basecolor.png");
    stbi_set_flip_vertically_on_load(true);
    stbi_uc* pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    if (!pixels) {
        throw std::runtime_error("failed to load test_texure.jpg!");
    }

    // 创建临时缓冲
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    mDevice->CreateBuffer(imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);

    // 图像数据拷贝到临时缓冲，并释放pixels
    void* data;
    vkMapMemory(mDevice->Get(), stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(mDevice->Get(), stagingBufferMemory);
    stbi_image_free(pixels);

    // 创建纹理图像
    VkImageCreateInfo imageInfo = vulkanInitializers::ImageCreateInfo(
        VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM,
        { static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1 },
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    mDevice->CreateImage(&imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        mTestTextureImage, mTestTextureImageMemory);

    // 转换image格式，undefined -> transferDst
    Device::ImageMemoryBarrierInfo imageBarrierInfo{};
    imageBarrierInfo.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageBarrierInfo.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageBarrierInfo.srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    imageBarrierInfo.srcAccessMask = 0;
    imageBarrierInfo.dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    imageBarrierInfo.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mDevice->TransitionImageLayout(mTestTextureImage, VK_IMAGE_ASPECT_COLOR_BIT, imageBarrierInfo);

    // 拷贝数据，从buffer到image
    mDevice->CopyBufferToImage(stagingBuffer, mTestTextureImage, texWidth, texHeight);

    // 转换image格式，transferDst -> shaderReadOnly
    imageBarrierInfo.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageBarrierInfo.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageBarrierInfo.srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    imageBarrierInfo.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageBarrierInfo.dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    imageBarrierInfo.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    mDevice->TransitionImageLayout(mTestTextureImage, VK_IMAGE_ASPECT_COLOR_BIT, imageBarrierInfo);

    // 清理临时缓冲
    vkDestroyBuffer(mDevice->Get(), stagingBuffer, nullptr);
    vkFreeMemory(mDevice->Get(), stagingBufferMemory, nullptr);

    // 创建imageView
    VkImageViewCreateInfo viewInfo = vulkanInitializers::ImageViewCreateInfo(mTestTextureImage,
        VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
    if (vkCreateImageView(mDevice->Get(), &viewInfo, nullptr, &mTestTextureImageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }
}

void DrawScenePbr::CleanUpTextures()
{
    vkDestroyImageView(mDevice->Get(), mTestTextureImageView, nullptr);
    vkDestroyImage(mDevice->Get(), mTestTextureImage, nullptr);
    vkFreeMemory(mDevice->Get(), mTestTextureImageMemory, nullptr);
}

void DrawScenePbr::CreateTextureSampler() {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    // 关闭各向异性滤波
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;

    // border color
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

    // true:[0,texWidth][0,texHeight]  false:[0,1][0,1]
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    // 比较：不开启， 主要用在shadow-map的PCF中
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    // mipmap
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(1);

    if (vkCreateSampler(mDevice->Get(), &samplerInfo, nullptr, &mTexureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texure sampler!");
    }
}

void DrawScenePbr::CleanUpTextureSampler() {
    vkDestroySampler(mDevice->Get(), mTexureSampler, nullptr);
}

void DrawScenePbr::UpdataUniformBuffer(float aspectRatio)
{
    UboMvpMatrix uboMvpMatrixs{};
    uboMvpMatrixs.model = glm::mat4(1.0f);

    uboMvpMatrixs.view = mCamera->GetView();
    uboMvpMatrixs.cameraPos = glm::inverse(uboMvpMatrixs.view) * glm::vec4(0.0, 0.0, 0.0, 1.0);

    mCamera->SetPerspective(aspectRatio, 0.1f, 100.0f, 45.0f);
    uboMvpMatrixs.proj = mCamera->GetProjection();
    uboMvpMatrixs.proj[1][1] *= -1;

    memcpy(mUboMvpMapped, &uboMvpMatrixs, sizeof(uboMvpMatrixs));

    UniformMaterial uboMaterial{};
    uboMaterial.albedo = glm::vec3(1.0, 0.5, 0.0);
    uboMaterial.roughness = 0.7f;
    uboMaterial.metallic = 1.0f;
    memcpy(mUboMaterialMapped, &uboMaterial, sizeof(uboMaterial));
}

void DrawScenePbr::UpdateDescriptorSets()
{
    VkDescriptorImageInfo sampleMainFbColorImageInfo = {
    mTexureSampler, mMainFbColorImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };
    std::vector<VkWriteDescriptorSet> presentDescriptorWrites(1);
    presentDescriptorWrites[0] = vulkanInitializers::WriteDescriptorSet(mDescriptorSetPresent,
        0, 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &sampleMainFbColorImageInfo);
    vkUpdateDescriptorSets(mDevice->Get(), presentDescriptorWrites.size(), presentDescriptorWrites.data(), 0, nullptr);
}

void DrawScenePbr::RecordPresentPass(VkCommandBuffer cmdBuf, const RenderInputInfo& input)
{
    // 启动Pass
    std::array<VkClearValue, 2> clearValues = {
        consts::CLEAR_COLOR_NAVY_FLT,
        consts::CLEAR_DEPTH_ONE_STENCIL_ZERO
    };
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = input.presentRenderPass;
    renderPassInfo.framebuffer = input.swapchanFb;
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = input.swapchainExtent;
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();
    vkCmdBeginRenderPass(cmdBuf, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // 绑定Pipeline
    vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelinePresent.pipeline);
    VkViewport viewport = { 0.0f, 0.0f, input.swapchainExtent.width, input.swapchainExtent.height, 0.0f, 1.0f };
    vkCmdSetViewport(cmdBuf, 0, 1, &viewport);
    VkRect2D scissor = { { 0, 0 }, input.swapchainExtent };
    vkCmdSetScissor(cmdBuf, 0, 1, &scissor);

    // 绑定DescriptorSet
    vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
        mPipelinePresent.layout,
        0, 1, &mDescriptorSetPresent,
        0, nullptr);

    //画图
    vkCmdDraw(cmdBuf, 4, 1, 0, 0);

    // 结束Pass
    vkCmdEndRenderPass(cmdBuf);
}
}   // namespace render
