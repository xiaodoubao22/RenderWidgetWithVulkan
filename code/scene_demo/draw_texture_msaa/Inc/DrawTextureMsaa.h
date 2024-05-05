#ifndef __DRAW_TEXTURE_MSAA_H__
#define __DRAW_TEXTURE_MSAA_H__

#include <vulkan/vulkan.h>

#include "SceneRenderBase.h"
#include "FrameworkHeaders.h"

namespace framework {
class DrawTextureMsaa : public SceneRenderBase {
public:
    DrawTextureMsaa() {}
    ~DrawTextureMsaa() {}

    void Init(const RenderInitInfo& initInfo) override;
    void CleanUp() override;
    std::vector<VkCommandBuffer>& RecordCommand(const RenderInputInfo& input) override;
    void OnResize(VkExtent2D newExtent) override;

private:

    void CreatePipelines();
    void CleanUpPipelines();

private:
    std::vector<VkCommandBuffer> mPrimaryCommandBuffers = {};
    VkCommandBuffer mCommandBuffer = VK_NULL_HANDLE;

    PipelineObjecs mPipeline = {};

};
}

#endif // __DRAW_TEXTURE_MSAA_H__
