#include "DrawSceneTest.h"

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

namespace render {
DrawSceneTest::DrawSceneTest()
{
    mMesh = new TestMesh;
    mCamera = new Camera;
}

DrawSceneTest::~DrawSceneTest()
{
    delete mCamera;
    delete mMesh;
}

void DrawSceneTest::Init(const RenderInitInfo& initInfo)
{
    if (!SceneRenderBase::InitCheck(initInfo)) {
        return;
    }

    std::string path = "../resource/models/viking_room.obj";
    mMesh->LoadFromFile(path);

    CreateRenderPasses();
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

void DrawSceneTest::CleanUp()
{
    CleanUpDescriptorPool();
    CleanUpTextureSampler();
    CleanUpTextures();
    CleanUpUniformBuffer();
    CleanUpIndexBuffer();
    CleanUpVertexBuffer();
    mDevice->FreeCommandBuffer(mCommandBuffer);
    CleanUpPipelines();
    CleanUpRenderPasses();
}

std::vector<VkCommandBuffer>& DrawSceneTest::RecordCommand(const RenderInputInfo& input)
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

    // 启动Pass
    std::array<VkClearValue, 2> clearValues = {
        consts::CLEAR_COLOR_NAVY_FLT,
        //consts::CLEAR_COLOR_WHITE_FLT,
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
    vkCmdBeginRenderPass(mCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // 绑定Pipeline
    vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline.pipeline);
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = input.swapchainExtent.width;
    viewport.height = input.swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(mCommandBuffer, 0, 1, &viewport);
    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = input.swapchainExtent;
    vkCmdSetScissor(mCommandBuffer, 0, 1, &scissor);

    // 绑定顶点缓冲
    std::vector<VkBuffer> vertexBuffers = { mVertexBuffer };
    std::vector<VkDeviceSize> offsets = { 0 };
    vkCmdBindVertexBuffers(mCommandBuffer, 0, 1, vertexBuffers.data(), offsets.data());

    // 绑定索引缓冲
    vkCmdBindIndexBuffer(mCommandBuffer, mIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

    // 绑定DescriptorSet
    vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        mPipeline.layout,
        0, 1, &mDescriptorSet,
        0, nullptr);

    //画图
    vkCmdDrawIndexed(mCommandBuffer, mMesh->GetIndexData().size(), 1, 0, 0, 0);
    //vkCmdDraw(commandBuffer, gVertices.size(), 1, 0, 0);

    // 结束Pass
    vkCmdEndRenderPass(mCommandBuffer);

    // 写入完成
    if (vkEndCommandBuffer(mCommandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }

    mPrimaryCommandBuffers.clear();
    mPrimaryCommandBuffers.emplace_back(mCommandBuffer);
    return mPrimaryCommandBuffers;
}

void DrawSceneTest::GetRequiredDeviceExtensions(std::vector<const char*>& deviceExt)
{
    return;
}

void DrawSceneTest::GetRequiredInstanceExtensions(std::vector<const char*>& deviceExt)
{
    return;
}

void DrawSceneTest::ProcessInputEnvent(const InputEventInfo& inputEnventInfo)
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

void DrawSceneTest::CreateRenderPasses()
{

}

void DrawSceneTest::CleanUpRenderPasses()
{

}

void DrawSceneTest::CreateVertexBuffer() {
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

void DrawSceneTest::CleanUpVertexBuffer() {
    // 销毁顶点缓冲区及显存
    vkDestroyBuffer(mDevice->Get(), mVertexBuffer, nullptr);
    vkFreeMemory(mDevice->Get(), mVertexBufferMemory, nullptr);
}

void DrawSceneTest::CreateIndexBuffer() {
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

void DrawSceneTest::CleanUpIndexBuffer() {
    vkDestroyBuffer(mDevice->Get(), mIndexBuffer, nullptr);
    vkFreeMemory(mDevice->Get(), mIndexBufferMemory, nullptr);
}

void DrawSceneTest::CreateUniformBuffer() {
    VkDeviceSize bufferSize = sizeof(UboMvpMatrix);

    mDevice->CreateBuffer(bufferSize,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        mUniformBuffer, mUniformBuffersMemory);

    vkMapMemory(mDevice->Get(), mUniformBuffersMemory, 0, bufferSize, 0, &mUniformBuffersMapped);
}

void DrawSceneTest::CleanUpUniformBuffer() {
    vkDestroyBuffer(mDevice->Get(), mUniformBuffer, nullptr);
    vkFreeMemory(mDevice->Get(), mUniformBuffersMemory, nullptr);
}

void DrawSceneTest::CreateDescriptorPool() {
    std::vector<VkDescriptorPoolSize> poolSizes = mPipeline.descriptorSizes;   // 池中各种类型的Descriptor个数

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 1;   // 池中最大能申请descriptorSet的个数
    if (vkCreateDescriptorPool(mDevice->Get(), &poolInfo, nullptr, &mDescriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void DrawSceneTest::CleanUpDescriptorPool() {
    vkDestroyDescriptorPool(mDevice->Get(), mDescriptorPool, nullptr);
}

void DrawSceneTest::CreateDescriptorSets() {
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts = mPipeline.descriptorSetLayouts;

    // 从池中申请descriptor set
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = mDescriptorPool;		// 从这个池中申请
    allocInfo.descriptorSetCount = descriptorSetLayouts.size();
    allocInfo.pSetLayouts = descriptorSetLayouts.data();
    if (vkAllocateDescriptorSets(mDevice->Get(), &allocInfo, &mDescriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    // 向descriptor set写入信息，个人理解目的是绑定buffer
    VkDescriptorBufferInfo uniformBufferInfo = { mUniformBuffer, 0, sizeof(UboMvpMatrix) };
    VkDescriptorImageInfo sampleImageInfo = { mTexureSampler, mTestTextureImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

    std::vector<VkWriteDescriptorSet> descriptorWrites(2);
    descriptorWrites[0] = vulkanInitializers::WriteDescriptorSet(mDescriptorSet,
        0, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &uniformBufferInfo);
    descriptorWrites[1] = vulkanInitializers::WriteDescriptorSet(mDescriptorSet,
        1, 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &sampleImageInfo);
    vkUpdateDescriptorSets(mDevice->Get(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

void DrawSceneTest::CreatePipelines()
{
    // create shader
    ShaderModuleFactory& shaderFactory = ShaderModuleFactory::GetInstance();
    std::vector<SpvFilePath> shaderFilePaths = {
        { setting::dirSpvFiles + std::string("DrawMesh.vert.spv"), VK_SHADER_STAGE_VERTEX_BIT },
        { setting::dirSpvFiles + std::string("DrawMesh.frag.spv"), VK_SHADER_STAGE_FRAGMENT_BIT },
    };
    ShaderProgram spDrawTexture = shaderFactory.CreateShaderProgramFromFiles(mDevice->Get(), shaderFilePaths);

    // descriptor size
    mPipeline.descriptorSizes = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
    };

    // descriptor layout
    mPipeline.descriptorSetLayouts.resize(1);
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings = {
        vulkanInitializers::DescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT),
        vulkanInitializers::DescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT),
    };
    VkDescriptorSetLayoutCreateInfo layoutInfo = vulkanInitializers::DescriptorSetLayoutCreateInfo(layoutBindings);
    if (vkCreateDescriptorSetLayout(mDevice->Get(), &layoutInfo, nullptr, &mPipeline.descriptorSetLayouts[0]) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    // pipeline layout
    VkPipelineLayoutCreateInfo layoutCreateInfo = vulkanInitializers::PipelineLayoutCreateInfo(
        mPipeline.descriptorSetLayouts, mPipeline.pushConstantRanges);
    if (vkCreatePipelineLayout(mDevice->Get(), &layoutCreateInfo, nullptr, &mPipeline.layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipline layout!");
    }

    // pipeline
    GraphicsPipelineConfigBase configInfo;
    configInfo.Fill();
    configInfo.SetVertexInputBindings({ Vertex3D::GetBindingDescription() });
    configInfo.SetVertexInputAttributes(Vertex3D::getAttributeDescriptions());
    configInfo.mDepthStencilState.depthTestEnable = VK_TRUE;
    configInfo.mDepthStencilState.depthWriteEnable = VK_TRUE;
    configInfo.mDepthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
    configInfo.mDepthStencilState.depthBoundsTestEnable = VK_FALSE;

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = configInfo.Populate(mPipeline.layout, mPresentRenderPass);
    pipelineCreateInfo.stageCount = spDrawTexture.shaderStageInfos.size();
    pipelineCreateInfo.pStages = spDrawTexture.shaderStageInfos.data();

    if (vkCreateGraphicsPipelines(mDevice->Get(), VK_NULL_HANDLE, 1,
        &pipelineCreateInfo, nullptr, &mPipeline.pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    // destroy shader
    shaderFactory.DestroyShaderProgram(spDrawTexture);
}

void DrawSceneTest::CleanUpPipelines()
{
    vkDestroyPipeline(mDevice->Get(), mPipeline.pipeline, nullptr);
    vkDestroyPipelineLayout(mDevice->Get(), mPipeline.layout, nullptr);
    for (auto setLayout : mPipeline.descriptorSetLayouts) {
        vkDestroyDescriptorSetLayout(mDevice->Get(), setLayout, nullptr);
    }
}

void DrawSceneTest::CreateTextures()
{
    // 读取图片
    int texWidth, texHeight, texChannels;
    std::string texturePath("../resource/textures/viking_room.png");
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

void DrawSceneTest::CleanUpTextures()
{
    vkDestroyImageView(mDevice->Get(), mTestTextureImageView, nullptr);
    vkDestroyImage(mDevice->Get(), mTestTextureImage, nullptr);
    vkFreeMemory(mDevice->Get(), mTestTextureImageMemory, nullptr);
}

void DrawSceneTest::CreateTextureSampler() {
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

void DrawSceneTest::CleanUpTextureSampler() {
    vkDestroySampler(mDevice->Get(), mTexureSampler, nullptr);
}

void DrawSceneTest::UpdataUniformBuffer(float aspectRatio)
{
    UboMvpMatrix uboMvpMatrixs{};
    uboMvpMatrixs.model = glm::mat4(1.0f);

    uboMvpMatrixs.view = mCamera->GetView();

    mCamera->SetPerspective(aspectRatio, 0.1f, 10.0f, 45.0f);
    uboMvpMatrixs.proj = mCamera->GetProjection();
    uboMvpMatrixs.proj[1][1] *= -1;

    memcpy(mUniformBuffersMapped, &uboMvpMatrixs, sizeof(uboMvpMatrixs));
}
}   // namespace render
