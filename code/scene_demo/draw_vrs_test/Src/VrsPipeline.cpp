#include "VrsPipeline.h"

#include "SceneDemoDefs.h"
#include "BufferCreator.h"

#include "Log.h"
#undef LOG_TAG
#define LOG_TAG "VrsPipeline"

namespace framework {

VrsPipeline::VrsPipeline()
{

}
VrsPipeline::~VrsPipeline()
{
}

void VrsPipeline::Init(Device* device)
{
    mDevice = device;

    //CreateRenderPass();
    CreatePipeline();
}

void VrsPipeline::CleanUp()
{
    CleanUpPipeline();
    //vkDestroyRenderPass(mDevice->Get(), mPassDrawVrsRegion, nullptr);
}

void VrsPipeline::CreateVrsImage(uint32_t width, uint32_t height)
{
    VkImageCreateInfo vrsImageInfo = vulkanInitializers::ImageCreateInfo(VK_IMAGE_TYPE_2D, VK_FORMAT_R8_UINT);
    vrsImageInfo.extent = { width / 8, height / 8, 1 };
    vrsImageInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR | VK_IMAGE_USAGE_SAMPLED_BIT;

    VmaAllocationCreateInfo imageAllocInfo = {};
    imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    if (vmaCreateImage(BufferCreator::GetInstance().GetAllocator(),
        &vrsImageInfo, &imageAllocInfo, &mVrsImage, &mVrsImageAllocation, nullptr) != VK_SUCCESS) {
        LOGE("failed to create texture image!");
    }

    VkImageViewCreateInfo viewInfo = vulkanInitializers::ImageViewCreateInfo(mVrsImage,
        VK_IMAGE_VIEW_TYPE_2D, vrsImageInfo.format, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
    if (vkCreateImageView(mDevice->Get(), &viewInfo, nullptr, &mVrsImageView) != VK_SUCCESS) {
        LOGE("failed to create texture image view!");
    }

    ImageMemoryBarrierInfo imageBarrierInfo{};
    imageBarrierInfo.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageBarrierInfo.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    imageBarrierInfo.srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    imageBarrierInfo.srcAccessMask = 0;
    imageBarrierInfo.dstStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    imageBarrierInfo.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    BufferCreator::GetInstance().TransitionImageLayout(mVrsImage, VK_IMAGE_ASPECT_COLOR_BIT, imageBarrierInfo);
}

void VrsPipeline::CleanUpVrsImage()
{
    vkDestroyImageView(mDevice->Get(), mVrsImageView, nullptr);
    vmaDestroyImage(BufferCreator::GetInstance().GetAllocator(), mVrsImage, mVrsImageAllocation);
}

void VrsPipeline::CreateRenderPass()
{
    // subpass
    VkAttachmentReference2 colorAttachmentRef =
        vulkanInitializers::AttachmentReference2(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    std::vector<VkSubpassDescription2> subpasses = {
        vulkanInitializers::SubpassDescription2(VK_PIPELINE_BIND_POINT_GRAPHICS, &colorAttachmentRef),
    };

    std::vector<VkSubpassDependency2> dependencys = { vulkanInitializers::SubpassDependency2(VK_SUBPASS_EXTERNAL, 0) };
    dependencys[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencys[0].srcAccessMask = 0;
    dependencys[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencys[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    std::vector<VkAttachmentDescription2> mAttachments2(1);
    mAttachments2[0] = vulkanInitializers::AttachmentDescription2(VK_FORMAT_R8_UINT);
    vulkanInitializers::AttachmentDescription2SetOp(mAttachments2[0],
        VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
    vulkanInitializers::AttachmentDescription2SetLayout(mAttachments2[0],
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    VkRenderPassCreateInfo2 renderPassInfo = vulkanInitializers::RenderPassCreateInfo2(mAttachments2, subpasses);
    vulkanInitializers::RenderPassCreateInfo2SetArray(renderPassInfo, dependencys);
    if (vkCreateRenderPass2(mDevice->Get(), &renderPassInfo, nullptr, &mPassDrawVrsRegion) != VK_SUCCESS) {
        LOGE("failed to create render pass!");
    }
}

void VrsPipeline::CreatePipeline()
{
    PipelineFactory& pipelineFactory = PipelineFactory::GetInstance();
    pipelineFactory.SetDevice(mDevice->Get());

    ShaderFileInfo computeVrsRegionShaderFile{};
    computeVrsRegionShaderFile.filePath = GetConfig().directory.dirSpvFiles + std::string("draw_vrs_region.comp.spv");
    computeVrsRegionShaderFile.stage = VK_SHADER_STAGE_COMPUTE_BIT;

    std::vector<VkDescriptorSetLayoutBinding> layoutBindings = {
        vulkanInitializers::DescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT),
        vulkanInitializers::DescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT),
    };

    std::vector<VkPushConstantRange> nullPushConstantRanges = {};

    mPipelineDrawVrsRegion = pipelineFactory.CreateComputePipeline(computeVrsRegionShaderFile, layoutBindings, nullPushConstantRanges);
}

void VrsPipeline::CleanUpPipeline()
{
    PipelineFactory& pipelineFactory = PipelineFactory::GetInstance();
    pipelineFactory.SetDevice(mDevice->Get());

    pipelineFactory.DestroyPipelineObjecst(mPipelineDrawVrsRegion);
}

} // namespace framework