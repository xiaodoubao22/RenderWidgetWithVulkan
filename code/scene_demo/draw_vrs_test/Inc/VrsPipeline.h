#ifndef __VRS_PIPELINE_H__
#define __VRS_PIPELINE_H__

#include "FrameworkHeaders.h"
#include "VmaUsage.h"

namespace framework {

class VrsPipeline {
public:
    VrsPipeline();
    ~VrsPipeline();

    void Init(Device* device);
    void CleanUp();

    void CreateVrsImage(VkImage mainFbColorImage, VkImageView mainFbColorImageView, uint32_t mainFbWidth, uint32_t mainFbHeight);
    void CleanUpVrsImage();

    void CmdPrepareShadingRate(VkCommandBuffer commandBuffer);
    void CmdAnalysisContent(VkCommandBuffer commandBUffer);

    PipelineObjecs& GetPipeline() {
        return mPipelineDrawVrsRegion;
    }

    PipelineObjecs& GetPipelineSmoothVrs() {
        return mPipelineSmoothVrs;
    }

    VkImage& GetVrsImage() {
        return mVrsImage;
    }

    VkImage& GetSmoothVrsImage() {
        return mSmoothVrsImage;
    }

    VkImageView& GetVrsImageView() {
        return mVrsImageView;
    }

    VkImageView& GetSmoothVrsImageView() {
        return mSmoothVrsImageView;
    }

private:
    void CreatePipeline();
    void CleanUpPipeline();

    void CreateDescriptorPool();
    void CreateDesciptorSets();
    void UpdateDescriptorSets(VkImageView mainFbColorAttachment, VkImageView vrsImageView, VkImageView smoothVrsImageView);

    void CreateSampler();
    void CleanUpSampler();

private:
    Device* mDevice = nullptr;
    VkImage mMainFbColorImage = VK_NULL_HANDLE;
    VkImageView mMainFbColorImageView = VK_NULL_HANDLE;
    uint32_t mMainFbWidth = 0;
    uint32_t mMainFbHeight = 0;

    PipelineObjecs mPipelineDrawVrsRegion = {};
    PipelineObjecs mPipelineSmoothVrs = {};

    VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet mDescriptorSetVrsComp = VK_NULL_HANDLE;
    VkDescriptorSet mDescriptorSetSmoothVrs = VK_NULL_HANDLE;

    // vrs image
    VmaAllocation mVrsImageAllocation = VK_NULL_HANDLE;
    VkImage mVrsImage = VK_NULL_HANDLE;
    VkImageView mVrsImageView = VK_NULL_HANDLE;

    VmaAllocation mSmoothVrsImageAllocation = VK_NULL_HANDLE;
    VkImage mSmoothVrsImage = VK_NULL_HANDLE;
    VkImageView mSmoothVrsImageView = VK_NULL_HANDLE;

    uint32_t mVrsImageWidth = 0;
    uint32_t mVrsImageHeight = 0;

    VkSampler mNearestSampler = VK_NULL_HANDLE;

};

}


#endif // !__VRS_PIPELINE_H__
