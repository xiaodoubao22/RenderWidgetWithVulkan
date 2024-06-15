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
    CreatePipeline();
}

void VrsPipeline::CleanUp()
{
    CleanUpPipeline();
}

void VrsPipeline::CreateVrsImage(uint32_t width, uint32_t height)
{
    // create image
    VkImageCreateInfo vrsImageInfo = vulkanInitializers::ImageCreateInfo(VK_IMAGE_TYPE_2D, VK_FORMAT_R8_UINT);
    vrsImageInfo.extent = { width / 8, height / 8, 1 };
    vrsImageInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR | VK_IMAGE_USAGE_SAMPLED_BIT;

    VmaAllocationCreateInfo imageAllocInfo = {};
    imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    if (vmaCreateImage(BufferCreator::GetInstance().GetAllocator(),
        &vrsImageInfo, &imageAllocInfo, &mVrsImage, &mVrsImageAllocation, nullptr) != VK_SUCCESS) {
        LOGE("failed to create texture image!");
    }
    if (vmaCreateImage(BufferCreator::GetInstance().GetAllocator(),
        &vrsImageInfo, &imageAllocInfo, &mSmoothVrsImage, &mSmoothVrsImageAllocation, nullptr) != VK_SUCCESS) {
        LOGE("failed to create texture image!");
    }

    // create image view
    VkImageViewCreateInfo viewInfo = vulkanInitializers::ImageViewCreateInfo(mVrsImage,
        VK_IMAGE_VIEW_TYPE_2D, vrsImageInfo.format, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
    if (vkCreateImageView(mDevice->Get(), &viewInfo, nullptr, &mVrsImageView) != VK_SUCCESS) {
        LOGE("failed to create texture image view!");
    }
    viewInfo.image = mSmoothVrsImage;
    if (vkCreateImageView(mDevice->Get(), &viewInfo, nullptr, &mSmoothVrsImageView) != VK_SUCCESS) {
        LOGE("failed to create texture image view!");
    }

    // transfer layout
    ImageMemoryBarrierInfo imageBarrierInfo{};
    imageBarrierInfo.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageBarrierInfo.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    imageBarrierInfo.srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    imageBarrierInfo.srcAccessMask = 0;
    imageBarrierInfo.dstStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    imageBarrierInfo.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    BufferCreator::GetInstance().TransitionImageLayout(mVrsImage, VK_IMAGE_ASPECT_COLOR_BIT, imageBarrierInfo);
    BufferCreator::GetInstance().TransitionImageLayout(mSmoothVrsImage, VK_IMAGE_ASPECT_COLOR_BIT, imageBarrierInfo);
}

void VrsPipeline::CleanUpVrsImage()
{
    vkDestroyImageView(mDevice->Get(), mSmoothVrsImageView, nullptr);
    vkDestroyImageView(mDevice->Get(), mVrsImageView, nullptr);
    vmaDestroyImage(BufferCreator::GetInstance().GetAllocator(), mSmoothVrsImage, mSmoothVrsImageAllocation);
    vmaDestroyImage(BufferCreator::GetInstance().GetAllocator(), mVrsImage, mVrsImageAllocation);
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

    ShaderFileInfo smoothVrsShaderFile{};
    smoothVrsShaderFile.filePath = GetConfig().directory.dirSpvFiles + std::string("smooth_shading_rate.comp.spv");
    smoothVrsShaderFile.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    mPipelineSmoothVrs = pipelineFactory.CreateComputePipeline(smoothVrsShaderFile, layoutBindings, nullPushConstantRanges);
}

void VrsPipeline::CleanUpPipeline()
{
    PipelineFactory& pipelineFactory = PipelineFactory::GetInstance();
    pipelineFactory.SetDevice(mDevice->Get());

    pipelineFactory.DestroyPipelineObjecst(mPipelineSmoothVrs);
    pipelineFactory.DestroyPipelineObjecst(mPipelineDrawVrsRegion);
}

} // namespace framework