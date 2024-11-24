#include "VrsPipeline.h"

#include "SceneDemoDefs.h"
#include "BufferCreator.h"

#include "Log.h"
#undef LOG_TAG
#define LOG_TAG "VrsPipeline"

namespace framework {

const static std::vector<DescriptorSetManager::DSEntry> VRS_ENTRIES= {
    {
        .updateType = DescriptorSetManager::TYPE_FIXED,
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
    },
    {
        .updateType = DescriptorSetManager::TYPE_FIXED,
        .binding = 2,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
    },
};

VrsPipeline::VrsPipeline()
{
    m_dsManager = std::make_shared<DescriptorSetManager>();
}
VrsPipeline::~VrsPipeline()
{
    m_dsManager = nullptr;
}

void VrsPipeline::Init(Device* device)
{
    mDevice = device;
    CreatePipeline();
    CreateSampler();
    CreateDescriptorPool();
    CreateDesciptorSets();

    DescriptorSetManager::InitInfo dsManagerInit = {
        .device = mDevice->Get(),
        .pEntries = VRS_ENTRIES.data(),
        .entriesSize = static_cast<uint32_t>(VRS_ENTRIES.size()),
    };
    m_dsManager->Init(&dsManagerInit);
}

void VrsPipeline::CleanUp()
{
    vkDestroyDescriptorPool(mDevice->Get(), mDescriptorPool, nullptr);
    CleanUpSampler();
    CleanUpPipeline();
}

void VrsPipeline::CreateVrsImage(VkImage mainFbColorImage, VkImageView mainFbColorImageView, uint32_t mainFbWidth, uint32_t mainFbHeight)
{
    // create image
    uint32_t vrsImageWidth = mainFbWidth / 8, vrsImageHeight = mainFbHeight / 8;
    VkImageCreateInfo vrsImageInfo = vulkanInitializers::ImageCreateInfo(VK_IMAGE_TYPE_2D, VK_FORMAT_R8_UINT);
    vrsImageInfo.extent = { vrsImageWidth, vrsImageHeight, 1 };
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

    mVrsImageWidth = vrsImageWidth;
    mVrsImageHeight = vrsImageHeight;

    // process main fb color attachment
    mMainFbColorImageView = mainFbColorImageView;
    mMainFbColorImage = mainFbColorImage;
    mMainFbWidth = mainFbWidth;
    mMainFbHeight = mainFbHeight;
    UpdateDescriptorSets(mMainFbColorImageView, mVrsImageView, mSmoothVrsImageView);
}

void VrsPipeline::CleanUpVrsImage()
{
    vkDestroyImageView(mDevice->Get(), mSmoothVrsImageView, nullptr);
    vkDestroyImageView(mDevice->Get(), mVrsImageView, nullptr);
    vmaDestroyImage(BufferCreator::GetInstance().GetAllocator(), mSmoothVrsImage, mSmoothVrsImageAllocation);
    vmaDestroyImage(BufferCreator::GetInstance().GetAllocator(), mVrsImage, mVrsImageAllocation);
}

void VrsPipeline::CmdPrepareShadingRate(VkCommandBuffer commandBuffer)
{
    ImageMemoryBarrierInfo imageBarrierInfo{};
    imageBarrierInfo.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    imageBarrierInfo.newLayout = VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR;
    imageBarrierInfo.srcStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    imageBarrierInfo.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    imageBarrierInfo.dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    imageBarrierInfo.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    mDevice->AddCmdPipelineBarrier(commandBuffer, mSmoothVrsImage, VK_IMAGE_ASPECT_COLOR_BIT, imageBarrierInfo);
}

void VrsPipeline::CmdAnalysisContent(VkCommandBuffer commandBuffer)
{
    // compute pass
    ImageMemoryBarrierInfo imageBarrierInfo{};
    imageBarrierInfo.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imageBarrierInfo.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    imageBarrierInfo.srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    imageBarrierInfo.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    imageBarrierInfo.dstStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    imageBarrierInfo.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    mDevice->AddCmdPipelineBarrier(commandBuffer, mMainFbColorImage, VK_IMAGE_ASPECT_COLOR_BIT, imageBarrierInfo);

    imageBarrierInfo.oldLayout = VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR;
    imageBarrierInfo.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    imageBarrierInfo.srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    imageBarrierInfo.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    imageBarrierInfo.dstStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    imageBarrierInfo.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    mDevice->AddCmdPipelineBarrier(commandBuffer, mSmoothVrsImage, VK_IMAGE_ASPECT_COLOR_BIT, imageBarrierInfo);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mPipelineDrawVrsRegion.pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
        mPipelineDrawVrsRegion.layout,
        0, 1, &mDescriptorSetVrsComp,
        0, nullptr);
    vkCmdDispatch(commandBuffer, mMainFbWidth / 16, mMainFbHeight / 16, 1);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mPipelineSmoothVrs.pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
        mPipelineSmoothVrs.layout,
        0, 1, &mDescriptorSetSmoothVrs,
        0, nullptr);
    vkCmdDispatch(commandBuffer, mVrsImageWidth / 16, mVrsImageHeight / 16, 1);
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

void VrsPipeline::CreateDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> poolSizes = {};   // 池中各种类型的Descriptor个数
    poolSizes.insert(poolSizes.end(), mPipelineDrawVrsRegion.descriptorSizes.begin(), mPipelineDrawVrsRegion.descriptorSizes.end());
    poolSizes.insert(poolSizes.end(), mPipelineSmoothVrs.descriptorSizes.begin(), mPipelineSmoothVrs.descriptorSizes.end());

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

void VrsPipeline::CreateDesciptorSets()
{
    VkDescriptorSetAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    allocInfo.descriptorPool = mDescriptorPool;		// 从这个池中申请

    // vrs compute pass
    allocInfo.descriptorSetCount = mPipelineDrawVrsRegion.descriptorSetLayouts.size();
    allocInfo.pSetLayouts = mPipelineDrawVrsRegion.descriptorSetLayouts.data();
    if (vkAllocateDescriptorSets(mDevice->Get(), &allocInfo, &mDescriptorSetVrsComp) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    // smooth vrs pass
    allocInfo.descriptorSetCount = mPipelineSmoothVrs.descriptorSetLayouts.size();
    allocInfo.pSetLayouts = mPipelineSmoothVrs.descriptorSetLayouts.data();
    if (vkAllocateDescriptorSets(mDevice->Get(), &allocInfo, &mDescriptorSetSmoothVrs) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }
}

void VrsPipeline::UpdateDescriptorSets(VkImageView mainFbColorAttachment, VkImageView vrsImageView, VkImageView smoothVrsImageView)
{
    VkDescriptorImageInfo texInputImageInfo = { mNearestSampler, mainFbColorAttachment, VK_IMAGE_LAYOUT_GENERAL };
    VkDescriptorImageInfo texVrsImageInfo = { mNearestSampler, vrsImageView, VK_IMAGE_LAYOUT_GENERAL };

    std::vector<VkWriteDescriptorSet> vrsDescriptorSetWrites = {
        vulkanInitializers::WriteDescriptorSet(mDescriptorSetVrsComp,
            0, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, &texInputImageInfo),
        vulkanInitializers::WriteDescriptorSet(mDescriptorSetVrsComp,
            1, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, &texVrsImageInfo),
    };

    vkUpdateDescriptorSets(mDevice->Get(), vrsDescriptorSetWrites.size(), vrsDescriptorSetWrites.data(), 0, nullptr);

    VkDescriptorImageInfo originVrsImageInfo = { mNearestSampler, vrsImageView, VK_IMAGE_LAYOUT_GENERAL };
    VkDescriptorImageInfo smoothVrsImageInfo = { mNearestSampler, smoothVrsImageView, VK_IMAGE_LAYOUT_GENERAL };

    std::vector<VkWriteDescriptorSet> smoothVrsDescriptorSetWrites = {
        vulkanInitializers::WriteDescriptorSet(mDescriptorSetSmoothVrs,
            0, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, &originVrsImageInfo),
        vulkanInitializers::WriteDescriptorSet(mDescriptorSetSmoothVrs,
            1, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, &smoothVrsImageInfo),
    };

    vkUpdateDescriptorSets(mDevice->Get(), smoothVrsDescriptorSetWrites.size(), smoothVrsDescriptorSetWrites.data(), 0, nullptr);
}

void VrsPipeline::CreateSampler()
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;
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
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(1);

    if (vkCreateSampler(mDevice->Get(), &samplerInfo, nullptr, &mNearestSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texure sampler!");
    }
}

void VrsPipeline::CleanUpSampler()
{
    vkDestroySampler(mDevice->Get(), mNearestSampler, nullptr);
}

} // namespace framework