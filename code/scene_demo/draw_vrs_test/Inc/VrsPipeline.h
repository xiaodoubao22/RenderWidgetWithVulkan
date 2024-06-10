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

    VkImage& GetVrsImage() {
        return mVrsImage;
    }

    VkImageView& GetVrsImageView() {
        return mVrsImageView;
    }

private:
    void CreateRenderPass();

    void CreatePipeline();
    void CleanUpPipeline();

private:
    Device* mDevice = nullptr;

    VkRenderPass mPassDrawVrsRegion = VK_NULL_HANDLE;

    PipelineObjecs mPipelineDrawVrsRegion = {};

    // vrs image
    VmaAllocation mVrsImageAllocation = VK_NULL_HANDLE;
    VkImage mVrsImage = VK_NULL_HANDLE;
    VkImageView mVrsImageView = VK_NULL_HANDLE;

};

}


#endif // !__VRS_PIPELINE_H__
