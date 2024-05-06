#include "DrawTextureMsaa.h"

#include <array>
#include "Log.h"
#include "VulkanInitializers.h"
#include "Utils.h"
#include "SceneDemoDefs.h"
#include "BufferCreator.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include <stb_image.h>

namespace framework {

void DrawTextureMsaa::Init(const RenderInitInfo& initInfo)
{
    if (!SceneRenderBase::InitCheck(initInfo)) {
        return;
    }

    mCommandBuffer = mDevice->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    CreatePipelines();
    CreateBuffers();
    CreateTextures();
    CreateTextureSampler();
    CreateDescriptorPool();
    CreateDescriptorSets();
}

void DrawTextureMsaa::CleanUp()
{
    CleanUpDescriptorPool();
    CleanUpTextureSampler();
    CleanUpTextures();
    CleanUpBuffers();
    CleanUpPipelines();
    if (mDevice != nullptr) {
        mDevice->FreeCommandBuffer(mCommandBuffer);
    }
}

std::vector<VkCommandBuffer>& DrawTextureMsaa::RecordCommand(const RenderInputInfo& input)
{
    vkResetCommandBuffer(mCommandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo = vulkanInitializers::CommandBufferBeginInfo(nullptr);
    if (vkBeginCommandBuffer(mCommandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("fiaile to begin recording command buffer!");
    }

    // 启动Pass
    static std::array<VkClearValue, 2> clearValues = {
        consts::CLEAR_COLOR_WHITE_FLT,
        consts::CLEAR_DEPTH_ONE_STENCIL_ZERO,
    };
    VkRenderPassBeginInfo renderPassInfo = vulkanInitializers::RenderPassBeginInfo(input.presentRenderPass, input.swapchanFb);
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = input.swapchainExtent;
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();
    vkCmdBeginRenderPass(mCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline.pipeline);

    VkViewport viewport = {
        .x = 0.0f, .y = 0.0f,
        .width = static_cast<float>(input.swapchainExtent.width),
        .height = static_cast<float>(input.swapchainExtent.height),
        .minDepth = 0.0, .maxDepth = 1.0f,
    };
    vkCmdSetViewport(mCommandBuffer, 0, 1, &viewport);
    VkRect2D scissor = { .offset = { 0, 0 }, .extent = input.swapchainExtent };
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
    vkCmdDrawIndexed(mCommandBuffer, mQuadIndices.size(), 1, 0, 0, 0);

    vkCmdEndRenderPass(mCommandBuffer);

    if (vkEndCommandBuffer(mCommandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }


    mPrimaryCommandBuffers = { mCommandBuffer };
    return mPrimaryCommandBuffers;
}

void DrawTextureMsaa::OnResize(VkExtent2D newExtent)
{

}

void DrawTextureMsaa::CreatePipelines()
{
    PipelineFactory& pipelineFactory = PipelineFactory::GetInstance();
    pipelineFactory.SetDevice(mDevice->Get());
    // create shader
    std::vector<ShaderFileInfo> shaderFilePaths = {
        { GetConfig().directory.dirSpvFiles + std::string("DrawTextureTest.vert.spv"), VK_SHADER_STAGE_VERTEX_BIT },
        { GetConfig().directory.dirSpvFiles + std::string("DrawTextureTest.frag.spv"), VK_SHADER_STAGE_FRAGMENT_BIT },
    };

    // descriptor layout
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings = {
        vulkanInitializers::DescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT),
    };

    // push constant
    std::vector<VkPushConstantRange> nullPushConstantRanges = {};

    // pipeline
    GraphicsPipelineConfigInfo configInfo{};
    configInfo.SetRenderPass(mPresentRenderPass);
    configInfo.SetVertexInputBindings({ Vertex2DColorTexture::GetBindingDescription() });
    configInfo.SetVertexInputAttributes(Vertex2DColorTexture::getAttributeDescriptions());
    configInfo.mMultisampleState.rasterizationSamples = GetConfig().presentFb.msaaSampleCount;

    mPipeline = pipelineFactory.CreateGraphicsPipeline(configInfo, shaderFilePaths, layoutBindings, nullPushConstantRanges);
}

void DrawTextureMsaa::CleanUpPipelines()
{
    vkDestroyPipeline(mDevice->Get(), mPipeline.pipeline, nullptr);
    vkDestroyPipelineLayout(mDevice->Get(), mPipeline.layout, nullptr);
    for (auto setLayout : mPipeline.descriptorSetLayouts) {
        vkDestroyDescriptorSetLayout(mDevice->Get(), setLayout, nullptr);
    }
}

void DrawTextureMsaa::CreateBuffers()
{
    BufferCreator& bufferCreator = BufferCreator::GetInstance();
    bufferCreator.SetDevice(mDevice);

    bufferCreator.CreateBufferFromSrcData(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, mQuadIndices.data(), mQuadIndices.size() * sizeof(mQuadIndices[0]),
        mIndexBuffer, mIndexBufferMemory);

    bufferCreator.CreateBufferFromSrcData(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, mQuadVertices.data(), mQuadVertices.size() * sizeof(mQuadVertices[0]),
        mVertexBuffer, mVertexBufferMemory);
}

void DrawTextureMsaa::CleanUpBuffers()
{
    vkDestroyBuffer(mDevice->Get(), mVertexBuffer, nullptr);
    vkFreeMemory(mDevice->Get(), mVertexBufferMemory, nullptr);
    vkDestroyBuffer(mDevice->Get(), mIndexBuffer, nullptr);
    vkFreeMemory(mDevice->Get(), mIndexBufferMemory, nullptr);
}

void DrawTextureMsaa::CreateTextures()
{
    // 读取图片
    int texWidth, texHeight, texChannels;
    std::string texturePath = "../resource/textures/test_texure.jpg";
    stbi_set_flip_vertically_on_load(true);
    stbi_uc* pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    if (!pixels) {
        throw std::runtime_error("failed to load test_texure.jpg!");
    }

    BufferCreator& bufferCreator = BufferCreator::GetInstance();
    bufferCreator.SetDevice(mDevice);

    VkImageCreateInfo imageInfo = vulkanInitializers::ImageCreateInfo(VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM);
    imageInfo.extent = { static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1 };
    imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    bufferCreator.CreateTextureFromSrcData(imageInfo, pixels, imageSize, mTestTextureImage, mTestTextureImageMemory);

    // 创建imageView
    VkImageViewCreateInfo viewInfo = vulkanInitializers::ImageViewCreateInfo(mTestTextureImage,
        VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
    if (vkCreateImageView(mDevice->Get(), &viewInfo, nullptr, &mTestTextureImageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }
}

void DrawTextureMsaa::CleanUpTextures()
{
    vkDestroyImageView(mDevice->Get(), mTestTextureImageView, nullptr);
    vkDestroyImage(mDevice->Get(), mTestTextureImage, nullptr);
    vkFreeMemory(mDevice->Get(), mTestTextureImageMemory, nullptr);
}

void DrawTextureMsaa::CreateTextureSampler()
{
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

void DrawTextureMsaa::CleanUpTextureSampler()
{
    vkDestroySampler(mDevice->Get(), mTexureSampler, nullptr);
}

void DrawTextureMsaa::CreateDescriptorPool()
{
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

void DrawTextureMsaa::CleanUpDescriptorPool()
{
    vkDestroyDescriptorPool(mDevice->Get(), mDescriptorPool, nullptr);
}

void DrawTextureMsaa::CreateDescriptorSets()
{
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
    std::vector<VkDescriptorImageInfo> imageInfos(1);
    imageInfos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfos[0].imageView = mTestTextureImageView;
    imageInfos[0].sampler = mTexureSampler;

    std::vector<VkWriteDescriptorSet> descriptorWrites(1);
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = mDescriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[0].dstArrayElement = 0;		// descriptors can be arrays
    descriptorWrites[0].descriptorCount = 1;	    // 想要更新多少个元素（从索引dstArrayElement开始）
    descriptorWrites[0].pBufferInfo = nullptr;			            //  ->
    descriptorWrites[0].pImageInfo = imageInfos.data();				//  -> 三选一
    descriptorWrites[0].pTexelBufferView = nullptr;				    //  ->
    vkUpdateDescriptorSets(mDevice->Get(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

}   // namespace framework