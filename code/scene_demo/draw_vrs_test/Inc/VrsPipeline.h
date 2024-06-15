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

    void CreateVrsImage(uint32_t width, uint32_t height);
    void CleanUpVrsImage();

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

private:
    Device* mDevice = nullptr;

    PipelineObjecs mPipelineDrawVrsRegion = {};
    PipelineObjecs mPipelineSmoothVrs = {};

    // vrs image
    VmaAllocation mVrsImageAllocation = VK_NULL_HANDLE;
    VkImage mVrsImage = VK_NULL_HANDLE;
    VkImageView mVrsImageView = VK_NULL_HANDLE;

    VmaAllocation mSmoothVrsImageAllocation = VK_NULL_HANDLE;
    VkImage mSmoothVrsImage = VK_NULL_HANDLE;
    VkImageView mSmoothVrsImageView = VK_NULL_HANDLE;

};

}


#endif // !__VRS_PIPELINE_H__
