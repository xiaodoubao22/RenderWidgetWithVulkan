#include "DrawTextureMsaa.h"

#include <array>
#include "Log.h"
#include "VulkanInitializers.h"
#include "Utils.h"
#include "SceneDemoDefs.h"

namespace framework {

void DrawTextureMsaa::Init(const RenderInitInfo& initInfo)
{
    if (!SceneRenderBase::InitCheck(initInfo)) {
        return;
    }

    mCommandBuffer = mDevice->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    CreatePipelines();
}
void DrawTextureMsaa::CleanUp()
{
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
        consts::CLEAR_COLOR_NAVY_FLT,
        consts::CLEAR_DEPTH_ONE_STENCIL_ZERO,
    };
    VkRenderPassBeginInfo renderPassInfo = vulkanInitializers::RenderPassBeginInfo(input.presentRenderPass, input.swapchanFb);
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = input.swapchainExtent;
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();
    vkCmdBeginRenderPass(mCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {
        .x = 0.0f, .y = 0.0f,
        .width = static_cast<float>(input.swapchainExtent.width),
        .height = static_cast<float>(input.swapchainExtent.height),
        .minDepth = 0.0, .maxDepth = 1.0f,
    };
    vkCmdSetViewport(mCommandBuffer, 0, 1, &viewport);
    VkRect2D scissor = { .offset = { 0, 0 }, .extent = input.swapchainExtent };
    vkCmdSetScissor(mCommandBuffer, 0, 1, &scissor);

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
}   // namespace framework