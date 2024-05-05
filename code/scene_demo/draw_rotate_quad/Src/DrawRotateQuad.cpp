#include "DrawRotateQuad.h"

#include <stdexcept>
#include <array>
#include <chrono>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "SceneDemoDefs.h"
#include "BufferCreator.h"

namespace framework {
DrawRotateQuad::DrawRotateQuad() {}

DrawRotateQuad::~DrawRotateQuad() {}

void DrawRotateQuad::Init(const RenderInitInfo& initInfo)
{
    if (!SceneRenderBase::InitCheck(initInfo)) {
        return;
    }
    
    mCommandBuffer = mDevice->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    CreateRenderPasses();
    CreatePipelines();
    CreateVertexBuffer();
    CreateIndexBuffer();
    CreateUniformBuffer();
    CreateDescriptorPool();
    CreateDescriptorSets();
}

void DrawRotateQuad::CleanUp()
{
    CleanUpDescriptorPool();
    CleanUpUniformBuffer();
    CleanUpIndexBuffer();
    CleanUpVertexBuffer();
    CleanUpPipelines();
    CleanUpRenderPasses();
    mDevice->FreeCommandBuffer(mCommandBuffer);
}

std::vector<VkCommandBuffer>& DrawRotateQuad::RecordCommand(const RenderInputInfo& input)
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
    vkCmdDrawIndexed(mCommandBuffer, mTriangleIndices.size(), 1, 0, 0, 0);

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

void DrawRotateQuad::CreateRenderPasses()
{

}

void DrawRotateQuad::CleanUpRenderPasses()
{

}

void DrawRotateQuad::CreateVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(mTriangleVertices[0]) * mTriangleVertices.size();

    BufferCreator& bufferCreator = BufferCreator::GetInstance();
    bufferCreator.SetDevice(mDevice);

    bufferCreator.CreateBufferFromSrcData(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, mTriangleVertices.data(), bufferSize,
        mVertexBuffer, mVertexBufferMemory);
}

void DrawRotateQuad::CleanUpVertexBuffer() {
    // 销毁顶点缓冲区及显存
    vkDestroyBuffer(mDevice->Get(), mVertexBuffer, nullptr);
    vkFreeMemory(mDevice->Get(), mVertexBufferMemory, nullptr);
}

void DrawRotateQuad::CreateIndexBuffer() {
    VkDeviceSize bufferSize = sizeof(mTriangleIndices[0]) * mTriangleIndices.size();

    BufferCreator& bufferCreator = BufferCreator::GetInstance();
    bufferCreator.SetDevice(mDevice);

    bufferCreator.CreateBufferFromSrcData(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, mTriangleIndices.data(), bufferSize,
        mIndexBuffer, mIndexBufferMemory);
}

void DrawRotateQuad::CleanUpIndexBuffer() {
    vkDestroyBuffer(mDevice->Get(), mIndexBuffer, nullptr);
    vkFreeMemory(mDevice->Get(), mIndexBufferMemory, nullptr);
}

void DrawRotateQuad::CreateUniformBuffer() {
    VkDeviceSize bufferSize = sizeof(UboMvpMatrix);

    BufferCreator& bufferCreator = BufferCreator::GetInstance();
    bufferCreator.SetDevice(mDevice);

    bufferCreator.CreateBuffer(bufferSize,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        mUniformBuffer, mUniformBuffersMemory);

    vkMapMemory(mDevice->Get(), mUniformBuffersMemory, 0, bufferSize, 0, &mUniformBuffersMapped);
}

void DrawRotateQuad::CleanUpUniformBuffer() {
    vkDestroyBuffer(mDevice->Get(), mUniformBuffer, nullptr);
    vkFreeMemory(mDevice->Get(), mUniformBuffersMemory, nullptr);
}

void DrawRotateQuad::CreateDescriptorPool() {
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

void DrawRotateQuad::CleanUpDescriptorPool() {
    vkDestroyDescriptorPool(mDevice->Get(), mDescriptorPool, nullptr);
}

void DrawRotateQuad::CreateDescriptorSets() {
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
    std::vector<VkDescriptorBufferInfo> bufferInfos(2);
    bufferInfos[0].buffer = mUniformBuffer;     // mvp矩阵的ubo
    bufferInfos[0].offset = 0;
    bufferInfos[0].range = sizeof(UboMvpMatrix);

    std::vector<VkWriteDescriptorSet> descriptorWrites(1);
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = mDescriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].dstArrayElement = 0;		// descriptors can be arrays
    descriptorWrites[0].descriptorCount = 1;	    // 想要更新多少个元素（从索引dstArrayElement开始）
    descriptorWrites[0].pBufferInfo = &bufferInfos[0];			//  ->
    descriptorWrites[0].pImageInfo = nullptr;					//  -> 三选一
    descriptorWrites[0].pTexelBufferView = nullptr;				//  ->
    vkUpdateDescriptorSets(mDevice->Get(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

void DrawRotateQuad::CreatePipelines()
{
    PipelineFactory& pipelineFactory = PipelineFactory::GetInstance();
    pipelineFactory.SetDevice(mDevice->Get());

    // create shader
    std::vector<ShaderFileInfo> shaderFilePaths = {
        { GetConfig().directory.dirSpvFiles + std::string("DrawTriangleTest.vert.spv"), VK_SHADER_STAGE_VERTEX_BIT },
        { GetConfig().directory.dirSpvFiles + std::string("DrawTriangleTest.frag.spv"), VK_SHADER_STAGE_FRAGMENT_BIT },
    };

    // descriptor layout
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings = {
        vulkanInitializers::DescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT),
    };

    // push constant
    std::vector<VkPushConstantRange> nullPushConstantRanges = {};

    // pipeline
    GraphicsPipelineConfigInfo configInfo;
    configInfo.SetRenderPass(mPresentRenderPass);
    configInfo.SetVertexInputBindings({ Vertex2DColor::GetBindingDescription() });
    configInfo.SetVertexInputAttributes(Vertex2DColor::getAttributeDescriptions());
    mPipeline = pipelineFactory.CreateGraphicsPipeline(configInfo, shaderFilePaths, layoutBindings, nullPushConstantRanges);

}

void DrawRotateQuad::CleanUpPipelines()
{
    vkDestroyPipeline(mDevice->Get(), mPipeline.pipeline, nullptr);
    vkDestroyPipelineLayout(mDevice->Get(), mPipeline.layout, nullptr);
    for (auto setLayout : mPipeline.descriptorSetLayouts) {
        vkDestroyDescriptorSetLayout(mDevice->Get(), setLayout, nullptr);
    }
}

void DrawRotateQuad::UpdataUniformBuffer(float aspectRatio)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UboMvpMatrix uboMvpMatrixs{};
    uboMvpMatrixs.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    uboMvpMatrixs.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    uboMvpMatrixs.proj = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 10.0f);
    uboMvpMatrixs.proj[1][1] *= -1;

    memcpy(mUniformBuffersMapped, &uboMvpMatrixs, sizeof(uboMvpMatrixs));
}
}   // namespace render



