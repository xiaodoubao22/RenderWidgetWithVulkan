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

    virtual void Init(const RenderInitInfo& initInfo) override;
    virtual void CleanUp() override;
    virtual std::vector<VkCommandBuffer>& RecordCommand(const RenderInputInfo& input) override;
    virtual void OnResize(VkExtent2D newExtent) override;

    virtual void GetRequiredDeviceExtensions(std::vector<const char*>& deviceExt) override;
    virtual void GetRequiredInstanceExtensions(std::vector<const char*>& deviceExt) override;

private:


private:
    std::vector<VkCommandBuffer> mPrimaryCommandBuffers = {};
    VkCommandBuffer mCommandBuffer = VK_NULL_HANDLE;

};
}

#endif // __DRAW_TEXTURE_MSAA_H__
