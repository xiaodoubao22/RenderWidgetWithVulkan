#ifndef __SCENE_RENDER_BASE__
#define __SCENE_RENDER_BASE__

#include <stdexcept>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "Device.h"
#include "GraphicsPipelineConfigInfo.h"
#include "TestMesh.h"

namespace framework {
struct RenderInitInfo {
    VkRenderPass presentRenderPass = VK_NULL_HANDLE;
    Device* device = nullptr;
    VkExtent2D swapchainExtent = {};
};

struct RenderInputInfo {
    VkRenderPass presentRenderPass = VK_NULL_HANDLE;
    VkFramebuffer swapchanFb = VK_NULL_HANDLE;
    VkExtent2D swapchainExtent = {};
};

struct InputEventInfo {
    bool leftPressFlag = false;
    bool rightPressFlag = false;
    bool middlePressFlag = false;
    float cursorX = 0.0;
    float cursorY = 0.0;
    int key = GLFW_KEY_UNKNOWN;
    int keyScancode = GLFW_KEY_UNKNOWN;
    int keyAction = GLFW_KEY_UNKNOWN;
};

class SceneRenderBase {
public:
    SceneRenderBase() {}
    ~SceneRenderBase() {}

    virtual void Init(const RenderInitInfo& initInfo) = 0;
    virtual void CleanUp() = 0;
    virtual std::vector<VkCommandBuffer>& RecordCommand(const RenderInputInfo& input) = 0;
    virtual void ProcessInputEvent(const InputEventInfo& inputEventInfo) {}
    virtual void OnResize(VkExtent2D newExtent) {}
    virtual void RequestPhysicalDeviceFeatures(PhysicalDevice* physicalDevice) {}

protected:
    bool InitCheck(const RenderInitInfo& initInfo)
    {
        if (initInfo.presentRenderPass == VK_NULL_HANDLE) {
            throw std::runtime_error("presentRenderPass is null");
            return false;
        }
        if (initInfo.device == nullptr) {
            throw std::runtime_error("device is null");
            return false;
        }
        mPresentRenderPass = initInfo.presentRenderPass;
        mDevice = initInfo.device;
        return true;
    }

protected:
    // external objects
    Device* mDevice = nullptr;
    VkRenderPass mPresentRenderPass = VK_NULL_HANDLE;

};
}   // namespace framework

#endif // __SCENE_RENDER_BASE__

